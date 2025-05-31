#include "log.h"

// 枚举类型
enum Type
{
    FILE,
    DIR,
    FREE
};
// inode structure on disk
struct dinode
{
    Type type;
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

    Type type;
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
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb->inodestart)

class Inode
{
private:
    itable itb;

    // 单例模式
    Logger &logger()
    {
        static Logger instance(*sb); // 延迟构造（函数首次调用时）
        return instance;
    }
    
    DiskDriver disk;
    superblock *sb;

public:
    Inode();

    // block allocation
    uint balloc();
    void bfree(uint);


    // inode allocation
    inode &iget(uint bn);
    inode &ialloc(Type type);
    void iupdate(inode &ip);
    void iput(inode &ip);
    uint bmap(inode &ip, uint bn);
    void itrunc(inode &ip);
};