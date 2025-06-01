#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "../src/log.h"

void testRW(Logger &logger)
{
    logger.beginOP();
    for (uint n = 50; n < 60; n++)
    {
        buf &b = logger.read(n);
        printf("read from%d: %s\n", n, b.data);
        char str[100];
        sprintf(str, "nigamahahahhaaaa%d", n);
        stpcpy((char *)b.data, str);
        logger.write(b);
        logger.relse(b);
    }
    logger.endOP();
}

int main()
{
    uint nmeta = 2 + LOGSIZE + NINODES;
    uint nbitmap = (DSIZE - nmeta) / (8 * BSIZE) + 1;
    superblock sb(FSMAGIC, DSIZE, DSIZE - nmeta - nbitmap,
                  NINODES, LOGSIZE, nbitmap,
                  2, 2 + LOGSIZE,
                  2 + LOGSIZE + NINODES);
    Logger logger(sb);
    testRW(logger);
}