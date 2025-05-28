#include "../src/buf.h"
#include "../src/disk.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>

// 测试读取和写入
void testReadWrite()
{
    DiskDriver disk;

    for (int i = 0; i < DSIZE / BSIZE; i++)
    {
        struct buf b;
        b.blockno = i;
        char str[100];
        sprintf(str, "Hello disk%d", i);
        printf("%s\n", str);
        // 测试写入
        strcpy((char *)b.data, str);
        b.blockno = i;
        disk.virtio_disk_rw(b, 1); // 1 = write
    }
    // struct buf b;
    // b.blockno = 0;
    // // 测试读取
    // disk.virtio_disk_rw(b, 0); // 0 = read
    // std::cout << "Read from disk: " << b.data << std::endl;

    for (int i = 0; i < DSIZE / BSIZE; i++)
    {
        struct buf b;
        
        b.blockno=i;
        // 测试读取
        disk.virtio_disk_rw(b, 0); // 0 = read

        std::cout << "Read from disk: " << b.data << std::endl;
        char str[100];
        sprintf(str, "Hello disk%d", i);
        assert(strcmp((char *)b.data, str) == 0);
    }
    std::cout << "Read/Write test passed!" << std::endl;
}

// 测试创建文件
void testCreateFile()
{
    DiskDriver disk;
}

int main()
{
    std::cout << "=== Testing DiskDriver ===" << std::endl;
    testReadWrite();
    testCreateFile();
    return 0;
}