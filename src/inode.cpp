#include "inode.h"
#include "dbg.h"
#include <cstring>

// 扫描bitmap，找到一个空闲的块，并将对应bit设为1
uint Inode::balloc()
{
    for (int i = 0; i < sb.nbitmap; i++)
    {
        buf &b = logger().read(sb.bmapstart + i);
        // 逐字节遍历
        for (int j = 0; j < BSIZE; j++)
        {
            if (b.data[j] != 0xff)
            {
                for (int k = 0; k < 8; k++)
                {
                    if ((b.data[j] & (1 << k)) == 0)
                    {
                        b.data[j] |= (1 << k);
                        logger().write(b);
                        logger().relse(b);
                        return NBB * i + j * 8 + k;
                    }
                }
            }
        }
    }
}

// 在bitmap中将指定块标记为空闲
void Inode::bfree(uint block_num)
{
    // 偏移的块数
    uint blockoff = block_num / NBB;
    // 偏移的字节
    uint byteoff = (block_num % NBB) / 8;
    // 偏移的bit
    uint bitoff = (block_num % NBB) % 8;
    buf &b = logger().read(sb.bmapstart + blockoff);
    uchar &byte = b.data[byteoff];
    byte &= ~(1 << bitoff);
    logger().write(b);
    logger().relse(b);
}

Inode::Inode()
{
    // read superblock
    buf b;
    b.blockno = 1;
    b.valid = true;
    disk.virtio_disk_rw(b, 0);
    superblock *ssb = (superblock *)b.data;
    this->sb = superblock(ssb->magic, ssb->size, ssb->nblocks,
                          ssb->ninodes, ssb->nlog, ssb->nbitmap,
                          ssb->logstart, ssb->inodestart, ssb->bmapstart);

    logger();
}

// 遍历itable，找到指定inum的缓存，若没有，返回一个空槽
inode &Inode::iget(uint inum)
{
    itb.lock.acquire();
    inode *in;
    for (int i = 0; i < ITBCAPACITY; i++)
    {
        if (!itb.inodes[i].valid)
        {
            in = &itb.inodes[i];
        }
        else if (itb.inodes[i].inum == inum)
        {
            in = &itb.inodes[i];
            in->ref++;
            itb.lock.release();
            return *in;
        }
    }
    in->inum = inum;
    in->ref = 1;
    in->valid = false;
    in->nlink = 0;
    itb.lock.release();
    return *in;
}

// 遍历磁盘上的inode，找到一个空槽，进行初始化
// 调用iget，将该inode缓存返回
inode &Inode::ialloc(InodeStructure::Type type)
{
    int inum;
    dinode *dip;
    for (inum = 1; inum < sb.ninodes; inum++)
    {
        // IBLOCK(inum, sb);
        buf &b = logger().read(IBLOCK(inum, sb.inodestart));
        dip = (dinode *)b.data + inum % IPB;
        if (dip->type == InodeStructure::FREE)
        { // a free inode
            std::memset(dip, 0, sizeof(*dip));
            dip->type = type;
            logger().write(b); // mark it allocated on the disk
            logger().relse(b);
            return iget(inum);
        }
        logger().relse(b);
    }
}

void Inode::initRoot()
{
    int inum = ROOTINO;
    dinode *dip;

    // IBLOCK(inum, sb);
    buf &b = logger().read(IBLOCK(inum, sb.inodestart));
    dip = (dinode *)b.data + inum % IPB;

    std::memset(dip, 0, sizeof(*dip));
    dip->type = InodeStructure::DIR;
    dip->nlink = 1;
    logger().write(b); // mark it allocated on the disk
    logger().relse(b);
}

// 复制一个修改的in-memory inode到磁盘。
//  每个调用ip->xxx修改后必须调用。
//  调用者必须持有ip->lock。
void Inode::iupdate(inode &ip)
{
    dinode *dip;

    buf &b = logger().read(IBLOCK(ip.inum, sb.inodestart));
    // 指针运算
    dip = (dinode *)b.data + ip.inum % IPB;
    dip->type = ip.type;
    dip->nlink = ip.nlink;
    dip->size = ip.size;
    memmove(dip->addrs, ip.addrs, sizeof(ip.addrs));
    logger().write(b);
    logger().relse(b);
}

// 减少引用次数，若引用次数为0
// 释放缓存，更新磁盘
void Inode::iput(inode &i)
{
    itb.lock.acquire();

    if (i.ref == 1 && i.valid && i.nlink == 0)
    {
        // inode has no links and no other references: truncate and free.

        // ip->ref == 1 means no other process can have ip locked,
        // so this acquiresleep() won't block (or deadlock).
        i.lock.acquire();

        itb.lock.release();

        itrunc(i);
        i.type = InodeStructure::FREE;
        iupdate(i);
        i.valid = 0;

        i.lock.release();

        itb.lock.acquire();
    }

    i.ref--;
    itb.lock.release();
}

//  将inode的块号映射到磁盘的块号
//  若该块不存在，则分配一个块，并更新磁盘
//  若该块存在，则返回该块的块号
//  返回0如果超过文件大小上限
uint Inode::bmap(inode &i, uint bn)
{
    uint addr, *a;

    if (bn < NDIRECT)
    {
        if ((addr = i.addrs[bn]) == 0)
        {
            addr = balloc();
            if (addr == 0)
                return 0;
            i.addrs[bn] = addr;
        }
        return addr;
    }
    bn -= NDIRECT;

    if (bn < NINDIRECT)
    {
        // 确保间接块已经分配
        if ((addr = i.addrs[NDIRECT]) == 0)
        {
            addr = balloc();
            if (addr == 0)
                return 0;
            i.addrs[NDIRECT] = addr;
        }
        buf &b = logger().read(addr);
        a = (uint *)b.data;
        if ((addr = a[bn]) == 0)
        {
            // 分配数据块
            addr = balloc();
            if (addr)
            {
                a[bn] = addr;
                logger().write(b);
            }
        }
        logger().relse(b);
        return addr;
    }
    dbg::panic("bmap: out of range");
}

void Inode::itrunc(inode &ip)
{
    int i, j;
    uint *a;

    for (i = 0; i < NDIRECT; i++)
    {
        if (ip.addrs[i])
        {
            bfree(ip.addrs[i]);
            ip.addrs[i] = 0;
        }
    }

    // 间接块存在
    if (ip.addrs[NDIRECT])
    {
        buf &b = logger().read(ip.addrs[NDIRECT]);
        a = (uint *)b.data;
        for (j = 0; j < NINDIRECT; j++)
        {
            if (a[j])
                bfree(a[j]);
        }
        logger().relse(b);
        bfree(ip.addrs[NDIRECT]);
        ip.addrs[NDIRECT] = 0;
    }

    ip.size = 0;
    iupdate(ip);
}

// 锁定inode
// 如果缓存中inode为ivalid
// 从磁盘读取进行同步
void Inode::ilock(inode &i)
{

    struct dinode *dip;

    i.lock.acquire();

    if (i.valid == 0)
    {
        buf &b = logger().read(IBLOCK(i.inum, sb.inodestart));
        dip = (dinode *)b.data + i.inum % IPB;
        i.type = dip->type;
        i.nlink = dip->nlink;
        i.size = dip->size;
        memmove(i.addrs, dip->addrs, sizeof(i.addrs));
        logger().relse(b);
        i.valid = true;
        if (i.type == InodeStructure::FREE)
            dbg::panic("Inode::ilock: no type");
    }
}

void Inode::iunlock(inode &i)
{
    if (!i.lock.holding())
        dbg::panic("Inode::iunlock: lock");

    i.lock.release();
}

int Inode::readi(struct inode &i, char *dst, uint off, uint n)
{
    uint tot, m;

    // 偏移量超过界限
    if (off > i.size || off + n < off)
        return 0;
    if (off + n > i.size)
        n = i.size - off;

    for (tot = 0; tot < n; tot += m, off += m, dst += m)
    {
        uint addr = bmap(i, off / BSIZE);
        if (addr == 0)
            break;
        buf &b = logger().read(addr);
        // 读取数据长度：m不能超过块边界且不能超过长度限制
        m = std::min(n - tot, BSIZE - off % BSIZE);

        std::memmove(dst, b.data + off % BSIZE, m);

        logger().relse(b);
    }
    return tot;
}

int Inode::writei(inode &i, char *src, uint off, uint n)
{
    uint tot, m;

    // 边界处理
    if (off > i.size || off + n < off)
        return -1;
    if (off + n > MAXFILE * BSIZE)
        return -1;

    for (tot = 0; tot < n; tot += m, off += m, src += m)
    {

        uint addr = bmap(i, off / BSIZE);
        if (addr == 0)
            break;
        buf &b = logger().read(addr);
        m = std::min(n - tot, BSIZE - off % BSIZE);
        std::memmove(b.data + (off % BSIZE), src, m);
        logger().write(b);
        logger().relse(b);
    }

    if (off > i.size)
        i.size = off;

    iupdate(i);

    return tot;
}

// 在目录中查找目录项
// 如果找到，返回inode并将poff设置为该项在目录偏移量
inode *Inode::dirlookup(inode &di, char *name, uint *poff)
{
    uint off, inum;
    dirent de;

    if (di.type != InodeStructure::DIR)
        dbg::panic("Inode::dirlookup: not DIR");

    for (off = 0; off < di.size; off += sizeof(de))
    {
        if (readi(di, (char *)&de, off, sizeof(de)) != sizeof(de))
            dbg::panic("Inode::dirlookup: read");
        if (de.inum == 0)
            continue;
        if (std::strcmp(name, de.name) == 0)
        {
            // entry matches path element
            if (poff)
                *poff = off;
            inum = de.inum;
            return &iget(inum);
        }
    }

    return nullptr;
}

void Inode::dirtree(inode &di, int cnt)
{
    uint off, inum;
    dirent de;

    if (di.type != InodeStructure::DIR)
        dbg::panic("Inode::dirlookup: not DIR");

    for (off = 0; off < di.size; off += sizeof(de))
    {
        if (readi(di, (char *)&de, off, sizeof(de)) != sizeof(de))
            dbg::panic("Inode::dirlookup: read");
        if (de.inum == 0)
            continue;
        for (int i = 0; i < cnt; i++)
            printf("   ");
        printf("%s \n", de.name);
        if(strcmp(de.name, ".")==0 || strcmp(de.name, "..")==0) continue;
        inode &in=iget(de.inum);
        ilock(in);
        if(in.type==InodeStructure::DIR) dirtree(in, cnt+1);
        iunlock(in);
        iput(in);

    }
}

// add dir entry
int Inode::dirlink(inode &dp, char *name, uint inum)
{
    int off;
    struct dirent de;
    struct inode *ip;

    // Check that name is not present.
    // if present, return -1
    if ((ip = dirlookup(dp, name, 0)) != 0)
    {
        iput(*ip);
        return -1;
    }

    // Look for an empty dirent.
    for (off = 0; off < dp.size; off += sizeof(de))
    {
        if (readi(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
            dbg::panic("Inode::dirlink: read");
        if (de.inum == 0)
            break;
    }

    strncpy(de.name, name, DIRSIZ);
    de.inum = inum;
    if (writei(dp, (char *)&de, off, sizeof(de)) != sizeof(de))
        return -1;

    return 0;
}

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char *
skipelem(char *path, char *name)
{
    char *s;
    int len;

    while (*path == '/')
        path++;
    if (*path == 0)
        return 0;
    s = path;
    while (*path != '/' && *path != 0)
        path++;
    len = path - s;
    if (len >= DIRSIZ)
        memmove(name, s, DIRSIZ);
    else
    {
        memmove(name, s, len);
        name[len] = 0;
    }
    while (*path == '/')
        path++;
    return path;
}

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls iput().
inode *Inode::namex(char *path, int nameiparent, char *name)
{
    struct inode *ip, *next;

    if (*path == '/')
        ip = &iget(ROOTINO);
    else
        dbg::panic("Inode::namex: bad path");

    while ((path = skipelem(path, name)) != 0)
    {
        ilock(*ip);
        if (ip->type != InodeStructure::DIR)
        {
            iunlock(*ip);
            iput(*ip);
            return 0;
        }
        if (nameiparent && *path == '\0')
        {
            // Stop one level early.
            iunlock(*ip);
            iput(*ip);
            return ip;
        }
        if ((next = dirlookup(*ip, name, 0)) == 0)
        {
            iunlock(*ip);
            iput(*ip);
            return 0;
        }
        iunlock(*ip);
        iput(*ip);
        ip = next;
    }
    if (nameiparent)
    {
        iput(*ip);
        return 0;
    }
    return ip;
}

inode *Inode::namei(char *path)
{
    char name[DIRSIZ];
    return namex(path, 0, name);
}

inode *Inode::nameiparent(char *path, char *name)
{
    return namex(path, 1, name);
}
