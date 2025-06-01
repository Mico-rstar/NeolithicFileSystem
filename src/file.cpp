#include "file.h"
#include "dbg.h"
#include <cstring>

int File::mkdir(char *path)
{
    in.logger().beginOP();
    inode *ip;
    if ((ip = create(path, file::FD_DIR)) == 0)
    {
        in.logger().endOP();
        return -1;
    }
    in.iunlock(*ip);
    in.iput(*ip);
    in.logger().endOP();

    return 0;
}
int File::unlink(char *path)
{
    struct inode *ip, *dp;
    struct dirent de;
    char name[DIRSIZ];
    uint off;

    in.logger().beginOP();
    if ((dp = in.nameiparent(path, name)) == 0)
    {
        in.logger().endOP();
        return -1;
    }

    in.ilock(*dp);

    // Cannot unlink "." or "..".
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        goto bad;

    if ((ip = in.dirlookup(*dp, name, &off)) == 0)
        goto bad;
    in.ilock(*ip);

    if (ip->nlink < 1)
        dbg::panic("unlink: nlink < 1");
    if (ip->type == InodeStructure::DIR && !isdirempty(ip))
    {
        in.iunlock(*ip);
        in.iput(*ip);
        goto bad;
    }

    memset(&de, 0, sizeof(de));
    if (in.writei(*dp, (char *)&de, off, sizeof(de)) != sizeof(de))
        dbg::panic("unlink: writei");
    if (ip->type == InodeStructure::DIR)
    {
        dp->nlink--;
        in.iupdate(*dp);
    }
    in.iunlock(*dp);
    in.iput(*dp);

    ip->nlink--;
    in.iupdate(*ip);
    in.iunlock(*ip);
    in.iput(*ip);

    in.logger().endOP();

    return 0;

bad:
    in.iunlock(*dp);
    in.iput(*dp);
    in.logger().endOP();
    return -1;
}

void File::tree(char *path)
{

    in.dirtree(*in.namei(path), 0);
}
fsstat File::fstat()
{
    return in.stat();
}
inode *File::create(char *path, short type)
{
    struct inode *ip, *dp;
    char name[DIRSIZ];

    if ((dp = in.nameiparent(path, name)) == 0)
        return 0;

    in.ilock(*dp);

    if ((ip = in.dirlookup(*dp, name, 0)) != 0)
    {
        // 目录中已经存在
        in.iunlock(*dp);
        in.iput(*dp);
        in.ilock(*ip);
        if (type == file::FD_FILE && (ip->type == InodeStructure::FILE))
            return ip;
        in.iunlock(*ip);
        in.iput(*ip);
        return 0;
    }
    // 不存在，分配inode
    if ((ip = &in.ialloc((InodeStructure::Type)type)) == 0)
    {
        in.iunlock(*dp);
        in.iput(*dp);
        return 0;
    }

    in.ilock(*ip);

    ip->nlink = 1;
    in.iupdate(*ip);

    if (type == file::FD_DIR)
    { // Create . and .. entries.
        // No ip->nlink++ for ".": avoid cyclic ref count.
        if (in.dirlink(*ip, ".", ip->inum) < 0 || in.dirlink(*ip, "..", dp->inum) < 0)
            goto fail;
    }

    if (in.dirlink(*dp, name, ip->inum) < 0)
        goto fail;

    if (type == file::FD_DIR)
    {
        // now that success is guaranteed:
        dp->nlink++; // for ".."
        in.iupdate(*dp);
    }

    in.iunlock(*dp);
    in.iput(*dp);

    return ip;

fail:
    // something went wrong. de-allocate ip.
    ip->nlink = 0;
    in.iupdate(*ip);
    in.iunlock(*ip);
    in.iput(*ip);
    in.iunlock(*dp);
    in.iput(*dp);
    return 0;
}

int File::isdirempty(inode *dp)
{
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de))
    {
        if (in.readi(*dp, (char *)&de, off, sizeof(de)) != sizeof(de))
            dbg::panic("isdirempty: readi");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

file *File::open(char *path, bool readable, bool writable)
{
    struct file *f;

    ftable.lock.acquire();
    in.logger().beginOP();
    for (f = ftable.file; f < ftable.file + NFILE; f++)
    {
        if (f->ref == 0)
        {
            f->ref = 1;
            f->readable = readable;
            f->writable = writable;
            f->ip = in.namei(path);
            if (f->ip == 0)
            {
                // 文件不存在,新建文件

                if ((f->ip = create(path, file::FD_FILE)) == 0)
                {
                    in.logger().endOP();
                    return 0;
                }
                in.iunlock(*f->ip);
                in.iput(*f->ip);
            }
            in.logger().endOP();
            ftable.lock.release();
            return f;
        }
    }
    in.logger().beginOP();

    ftable.lock.release();
    return 0;
}

void File::close(file *f)
{
    struct file ff;

    ftable.lock.acquire();
    if (f->ref < 1)
        dbg::panic("fileclose");
    if (--f->ref > 0)
    {
        ftable.lock.release();
        return;
    }
    ff = *f;
    f->ref = 0;
    f->type = file::FD_NONE;
    ftable.lock.release();

    in.logger().beginOP();
    in.iput(*ff.ip);
    in.logger().endOP();
}

int File::read(file *f, char *addr, uint n)
{
    int r = 0;

    if (f->readable == 0)
        return -1;

    in.ilock(*f->ip);
    if ((r = in.readi(*f->ip, addr, f->off, n)) > 0)
        f->off += r;
    in.iunlock(*f->ip);

    return r;
}

int File::write(file *f, char *addr, uint n)
{
    int r, ret = 0;

    if (f->writable == 0)
        return -1;

    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
    int i = 0;
    while (i < n)
    {
        int n1 = n - i;
        if (n1 > max)
            n1 = max;

        in.logger().beginOP();
        in.ilock(*f->ip);
        if ((r = in.writei(*f->ip, addr + i, f->off, n1)) > 0)
            f->off += r;
        in.iunlock(*f->ip);
        in.logger().endOP();

        if (r != n1)
        {
            // error from writei
            break;
        }
        i += r;
    }
    ret = (i == n ? n : -1);

    return ret;
}
