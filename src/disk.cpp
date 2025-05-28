#include <fstream>
#include <vector>
#include <iostream>
#include <stdio.h>

#include "disk.h"
#include "fs.h"
#include "buf.h"

void DiskDriver::read(const char *filename, struct buf &b)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error: diskdata.bin does not exist or cannot be opened for reading." << std::endl;
        return;
    }
    file.seekg(b.blockno * BSIZE, std::ios::beg);
    file.read((char *)b.data, BSIZE);
    file.close();
}

void DiskDriver::write(const char *filename, buf &b)
{
    std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file)
    {
        std::cerr << "Failed to open img for writing." << std::endl;
        return;
    }

    file.seekp(b.blockno * BSIZE, std::ios::beg);
    file.write((char *)b.data, BSIZE);
    file.close();
}

void DiskDriver::createFile(const char *filename, size_t sizeBytes)
{
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to open img." << std::endl;
        return;
    }
    // 创建并填充指定大小的空数据
    char fill = '\0';
    for (int i = 0; i < sizeBytes; i++)
    {
        outFile.write(&fill, 1);
    }
    outFile.close();
}

// 0: read
// 1: write
void DiskDriver::virtio_disk_rw(buf &b, int write)
{
    if (write)
        this->write(IMGPATH, b);
    else
        this->read(IMGPATH, b);
}

DiskDriver::DiskDriver()
{
    // create fs.img
    // alloc 8 MB
    std::ifstream file(IMGPATH, std::ios::binary);
    if (!file)
    {
        // fs.img not exist
        createFile(IMGPATH, DSIZE);
    }
}
