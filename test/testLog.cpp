#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "../src/log.h"

int main()
{
    superblock sb(FSMAGIC, DSIZE, DSIZE / BSIZE, 0, LOGSIZE, 2, 2 + LOGSIZE, 2 + LOGSIZE + 1);
    Logger logger(sb);
}