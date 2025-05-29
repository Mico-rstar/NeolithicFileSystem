#pragma once

#include <unordered_map>
#include <queue>
#include <vector>

#include "types.h"
#include "buf.h"
#include "sleeplock.h"

// 缓冲区数据结构
// 堆：存储buf
// 哈希表：blockno -> index in heap
// LRU
class Bcache
{
private:
    SleepLock lock;

    std::vector<buf *> bheap;           // buf heap
    std::unordered_map<int, uint> imap; // index map

    void siftUp(size_t index);
    void siftDown(size_t index);
    void swap(uint i, uint j);
    void heapify(const std::vector<buf *> &bufs);

public:
    Bcache(buf bufs[], size_t size);
    buf &bget(uint blockno);

    void brelease(buf &buf);
};