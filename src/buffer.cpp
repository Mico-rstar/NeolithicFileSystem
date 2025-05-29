#include "buffer.h"
#include "dbg.h"

Buffer::Buffer() : bcache(bufs, NBUF), disk()
{
}

buf &Buffer::bread(uint blockno)
{
    buf &b = bcache.bget(blockno);
    if (!b.valid)
    {
        this->disk.virtio_disk_rw(b, 0);
        b.valid = true;
    }
    return b;
}

void Buffer::bwrite(buf &b)
{
    if (!b.lock.holding())
        dbg::panic("bwrite: lock");

    this->disk.virtio_disk_rw(b, 1);
}

void Buffer::brelse(buf &b)
{
    if (!b.lock.holding())
        dbg::panic("brelse: lock");
    bcache.brelease(b);
}
