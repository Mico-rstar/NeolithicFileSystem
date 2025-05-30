
#include "src/fs.h"
#include "src/buffer.h"


void initSb(superblock &sb)
{
    sb.magic = FSMAGIC;
    sb.size = DSIZE;
    sb.nblocks = DSIZE / BSIZE;
    sb.ninodes = 0;
    sb.nlog = LOGSIZE;
    sb.logstart = 2;
    sb.inodestart = 0;
    sb.bmapstart = 0;
}

void initDisk(Buffer &buffer, superblock& sb)
{
    
    buf &b=buffer.bread(1);
}

int main()
{
    superblock sb;
    initSb(sb);

    Buffer buffer;
}