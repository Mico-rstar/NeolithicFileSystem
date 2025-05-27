#include <fstream>
#include <vector>
#include <iostream>

#include "disk.h"
#include "fs.h"

void DiskDriver::createFile(const char *filename, size_t sizeBytes)
{
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
    {
        std::cerr << "Failed to open file." << std::endl;
        return;
    }
    // 创建并填充指定大小的空数据
    std::vector<char> emptyData(sizeBytes, '\0');
    outFile.write(emptyData.data(), emptyData.size());
    outFile.close();
}

DiskDriver::DiskDriver()
{
    // create fs.img
    // alloc 8 MB
    createFile(IMGPATH, DSIZE);
}
