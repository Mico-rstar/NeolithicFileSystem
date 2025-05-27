
class DiskDriver
{
public:
    void virtio_disk_rw(struct buf *b, int write);
};