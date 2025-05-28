
#pragma once

#include "types.h"
#include "fs.h"
#include "sleeplock.h"

struct buf
{
  SleepLock lock;
  int valid; // has data been read from disk?
  uint blockno;
  // struct sleeplock lock;
  uint refcnt;
  uchar data[BSIZE] = {0};
};
