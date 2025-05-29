#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "../src/bcache.h"
#include "../src/buffer.h"

using namespace std;

buf bufs[NBUF];

void testBuild()
{

    Bcache bcache(bufs, NBUF);
}

void tesBget()
{
    Bcache bcache(bufs, NBUF);
    for (int i = 0; i < 100; i++)
    {
        uint blockno = i % 30;
        buf &b = bcache.bget(blockno);
        printf("ref=%d, valid=%d, blockno=%d\n", b.refcnt, b.valid, b.blockno);
        bcache.brelease(b);
        printf("r------------------\n");
    }
}

void testBuffer()
{
    Buffer buffer;
    uint i = 3;
    buf &b = buffer.bread(i);
    printf("read from %d block: %s\n", i, b.data);
    strcpy((char *)b.data, "Hello hahha");
    buffer.bwrite(b);
    buf &newb = buffer.bread(i);
    printf("read from %d block: %s\n", i, newb.data);
    buffer.brelse(b);
    buffer.brelse(newb);
}

int main()
{
    testBuffer();
    return 0;
}