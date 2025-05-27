
#pragma once


#include "types.h"
#include "fs.h"

struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint blockno;
  //struct sleeplock lock;
  uint refcnt;
  uchar data[BSIZE];
};

