#pragma once

#include "log.h"

namespace InodeStructure
{
    // 枚举类型
    enum Type
    {
        FREE,
        DIR,
        FILE

    };
};

// inode structure on disk
struct dinode
{
    InodeStructure::Type type;
    uint nlink;
    uint size;
    uint addrs[NDIRECT + 1]; // 最后一个块是间接块
};

// inode structure on mem
struct inode
{
    SleepLock lock;
    uint inum; // inumber
    uint ref;
    bool valid;

    InodeStructure::Type type;
    uint nlink;
    uint size;
    uint addrs[NDIRECT + 1]; // 最后一个块是间接块
};

const uint ITBCAPACITY = NINODES / 5; // itable capacity

// inode缓存，容量设为disk中总量的1/5
struct itable
{
    SleepLock lock;
    inode inodes[ITBCAPACITY];
};

// Inodes per block.
#define IPB (BSIZE / sizeof(dinode))

// Block containing inode i
#define IBLOCK(i, start) ((i) / IPB + start)

class Inode
{
private:
    itable itb;

    DiskDriver disk;
    // superblock *sb;
    superblock sb;

public:
    Inode();
    // 单例模式
    Logger &logger()
    {
        static Logger instance(sb); // 延迟构造（函数首次调用时）
        return instance;
    }
    // block allocation
    uint balloc();
    void bfree(uint);

    // inode allocation
    inode &iget(uint bn);
    inode &ialloc(InodeStructure::Type type);
    void iupdate(inode &ip);
    void iput(inode &ip);
    uint bmap(inode &ip, uint bn);
    void itrunc(inode &ip);
    void ilock(inode &i);
    void iunlock(inode &i);

    int readi(inode &i, char *dst, uint off, uint n);
    int writei(inode &i, char *src, uint off, uint n);

    // dir operations
    inode *dirlookup(struct inode &di, char *name, uint *poff);
    void dirtree(struct inode &di, int cnt);

    int dirlink(inode &di, char *name, uint inum);
    inode *namex(char *path, int nameiparent, char *name);

    inode *namei(char *path);
    inode *nameiparent(char *path, char *name);

    void initRoot();
};