#include "../src/buf.h"
#include "../src/disk.h"
#include <cassert>
#include <iostream>

// 测试读取和写入
void testReadWrite() {
    // DiskDriver disk;
    // struct buf b;

    // // 测试写入
    // b.data = "Hello, Disk!";
    // b.size = strlen(b.data) + 1; // +1 for null terminator
    // disk.virtio_disk_rw(b, 1);   // 1 = write

    // // 测试读取
    // char readBuffer[256] = {0};
    // b.data = readBuffer;
    // disk.virtio_disk_rw(b, 0);   // 0 = read

    // std::cout << "Read from disk: " << b.data << std::endl;
    // assert(strcmp(b.data, "Hello, Disk!") == 0);
    // std::cout << "Read/Write test passed!" << std::endl;
}

// 测试创建文件
void testCreateFile() {
    DiskDriver disk;
}

int main() {
    std::cout << "=== Testing DiskDriver ===" << std::endl;
    // testReadWrite();
    testCreateFile();
    return 0;
}