#include "inode.h"

// 扫描bitmap，找到一个空闲的块，并将对应bit设为1
uint Inode::balloc()
{
    for (int i = 0; i < sb->nbitmap; i++)
    {
        buf &b = logger().read(sb->bmapstart + i);
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
    buf &b = logger().read(sb->bmapstart + blockoff);
    uchar byte = b.data[byteoff];
    byte &= ~(1 << bitoff);
}

Inode::Inode()
{
    // read superblock
    buf b;
    b.blockno = 1;
    b.valid = true;
    disk.virtio_disk_rw(b, 0);
    sb = (superblock *)b.data;

    logger();
}

// 遍历itable，找到指定inum的缓存，若没有，返回一个空槽
inode &Inode::iget(uint inum)
{
    inode *in;
    for (int i = 0; i < ITBCAPACITY; i++)
    {
        if (!itb.inodes[i].valid)
            in = &itb.inodes[i];
        else if (itb.inodes[i].inum == inum)
        {
            return itb.inodes[i];
        }
    }
    return *in;
}

// 遍历磁盘上的inode，找到一个空槽，进行初始化
// 调用iget，将该inode缓存返回
inode &Inode::ialloc(Type type)
{
    
}
