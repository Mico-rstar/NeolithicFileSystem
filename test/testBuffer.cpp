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
    for (uint i = 0; i < 100; i++)
    {
        buf &b = buffer.bread(i);
        printf("1. read from %d block: %s\n", i, b.data);
        char str[100];
        sprintf(str, "woaigiegie%d", i);
        strcpy((char *)b.data, str);
        buffer.bwrite(b);
        buffer.brelse(b);
        buffer.bread(b.blockno);
        printf("2. read from %d block: %s\n", i, b.data);
        buffer.brelse(b);
    }
}

int main()
{
    testBuffer();
    return 0;
}