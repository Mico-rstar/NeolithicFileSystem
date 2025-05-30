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
        dbg::panic("Buffer::bwrite: lock");

    this->disk.virtio_disk_rw(b, 1);
}

void Buffer::brelse(buf &b)
{
    if (!b.lock.holding())
        dbg::panic("Buffer::brelse: lock");
    bcache.brelease(b);
}

void Buffer::bpin(buf &b)
{
    if (!b.lock.holding())
        dbg::panic("Buffer.bpin: lock");

    bcache.bpin(b);
}

void Buffer::bunpin(buf &b)
{
    if (!b.lock.holding())
        dbg::panic("Buffer.bpin: lock");

    bcache.bunpin(b);
}
