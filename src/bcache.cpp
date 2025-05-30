#include <utility>
#include "bcache.h"
#include "dbg.h"

void Bcache::siftUp(size_t index)
{

    while (index > 0)
    {
        size_t parent = (index - 1) / 2;
        if (bheap[index]->refcnt < bheap[parent]->refcnt)
        {
            // 更新哈希表
            this->swap(index, parent);
            index = parent;
        }
        else
        {
            break;
        }
    }
}

void Bcache::siftDown(size_t index)
{

    while (true)
    {
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t smallest = index;

        if (left < bheap.size() && bheap[left]->refcnt < bheap[smallest]->refcnt)
            smallest = left;
        if (right < bheap.size() && bheap[right]->refcnt < bheap[smallest]->refcnt)
            smallest = right;

        if (smallest != index)
        {
            // 更新哈希表
            // printf("%d. %d\n", index, smallest);
            this->swap(index, smallest);
            index = smallest;
        }
        else
            break;
    }
}

void Bcache::swap(uint i, uint j)
{
    
    std::swap(bheap[i], bheap[j]);

    // if (bheap[i]->blockno != -1)
        imap[bheap[i]->blockno] = i;
    // if (bheap[i]->blockno != -1)
        imap[bheap[j]->blockno] = j;
}

void Bcache::heapify(const std::vector<buf *> &bufs)
{
    for (int i = bufs.size() / 2 - 1; i >= 0; i--)
    {
        siftDown(i);
    }
}

// bufs的生命周期应该覆盖bcache
Bcache::Bcache(buf bufs[], size_t size) : bheap(size), imap(1.5 * size)
{
    this->lock.acquire();
    // 复制
    for (size_t i = 0; i < size; i++)
    {
        bheap[i] = &bufs[i];
        if (bufs[i].valid)
        {
            if (!imap[bufs[i].blockno])
                dbg::panic("Bcache: imap");

            imap[bufs[i].blockno] = i;
        }
    }

    // 建堆
    heapify(bheap);

    this->lock.release();
}

//
buf &Bcache::bget(uint blockno)
{
    this->lock.acquire();

    // 查看块是否已经在缓存里
    if (imap.count(blockno))
    {
        if (!imap.count(blockno))
            dbg::panic("bget: imap");
        buf *b = bheap[imap[blockno]];

        b->refcnt++;
        // update
        siftDown(imap[blockno]);

        if (!b->valid)
        {
            dbg::panic("bget: valid");
        }
        this->lock.release();
        b->lock.acquire();
        return *b;
    }

    // LRU
    // 没有空余缓存，需要覆盖最少使用（引用计数为0）的块
    if (!bheap[0]->refcnt)
    {
        buf *b = bheap[0];
        b->valid = false;
        b->refcnt = 1;
        b->blockno = blockno;
        // update
        imap[blockno] = 0;
        siftDown(0);

      

        this->lock.release();
        b->lock.acquire();
        return *b;
    }
    else
    {
        dbg::panic("not enough buf");
    }
}

void Bcache::brelease(buf &b)
{
    if (!b.lock.holding())
        dbg::panic("brelease: lock");
    if (b.refcnt <= 0)
        dbg::panic("brelease: refcnt");
    b.refcnt--;
    if (!imap.count(b.blockno))
    {
        dbg::panic("brelease: imap");
    }
    // update
    this->lock.acquire();
    siftUp(imap[b.blockno]);
    this->lock.release();
    b.lock.release();
}
