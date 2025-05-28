#pragma once
#include "types.h"

// 模拟磁盘驱动
class DiskDriver
{
private:
    void read(const char *filename, struct buf &b);
    void write(const char *filename, struct buf &b);
    void createFile(const char *filename, size_t sizeBytes);

public:
    void virtio_disk_rw(struct buf &b, int write);
    DiskDriver();
};