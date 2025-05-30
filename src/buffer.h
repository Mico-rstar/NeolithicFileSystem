#pragma once

#include "bcache.h"
#include "disk.h"

class Buffer
{
private:
    buf bufs[NBUF];
    Bcache bcache;
    DiskDriver disk;

public:
    Buffer();
    buf &bread(uint);
    void bwrite(buf &);
    void brelse(buf &);
    void bpin(buf &);
    void bunpin(buf &);
};