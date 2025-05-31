
#pragma once

#include "types.h"
#include "fs.h"
#include "sleeplock.h"

struct buf
{
  SleepLock lock;
  int blockno;
  // struct sleeplock lock;
  uint refcnt;
  bool valid;
  uchar data[BSIZE] = {0};

  struct Compare
  {
    bool operator()(const buf &a, const buf &b)
    {
      return a.refcnt > b.refcnt; // 小根堆
    }
  };

  buf()
  {
    // 未分配任何块
    blockno = -1;
    refcnt = 0;
    valid = false;
  }


  void print() const
  {
    printf("blockno = %d, refcnt = %d, valid = %d, data = %s, locked=%d\n", blockno, refcnt, valid, data, lock.holding());
  }
};
