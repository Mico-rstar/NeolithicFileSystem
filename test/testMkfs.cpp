#include <cstring>

#include "../src/fs.h"
#include "../src/buffer.h"

/**
 * @brief 将指定内存区域的指定位全部设置为1
 * @param ptr 起始地址
 * @param start_bit 起始位（0-based）
 * @param num_bits 要设置的位数
 */
void set_bits_to_one(void *ptr, size_t start_bit, size_t num_bits)
{
    auto *byte_ptr = static_cast<unsigned char *>(ptr);

    // 计算起始字节和起始位在字节内的偏移
    size_t start_byte = start_bit / 8;
    size_t offset_in_byte = start_bit % 8;

    // 处理起始字节的非对齐部分
    if (offset_in_byte != 0)
    {
        unsigned char mask = 0xFF << offset_in_byte;
        size_t bits_to_set = std::min(8 - offset_in_byte, num_bits);
        mask &= (0xFF >> (8 - bits_to_set)); // 确保不超出num_bits范围
        byte_ptr[start_byte] |= mask;

        num_bits -= bits_to_set;
        start_byte++;
    }

    // 设置完整的字节（如果剩余位数>=8）
    if (num_bits >= 8)
    {
        size_t full_bytes = num_bits / 8;
        memset(byte_ptr + start_byte, 0xFF, full_bytes);
        start_byte += full_bytes;
        num_bits %= 8;
    }

    // 处理剩余的不足一个字节的位
    if (num_bits > 0)
    {
        unsigned char mask = 0xFF >> (8 - num_bits);
        byte_ptr[start_byte] |= mask;
    }
}

void bzero(Buffer &buffer, uint blockno)
{
    buf &b = buffer.bread(blockno);
    std::memset(b.data, 0, BSIZE);
    buffer.bwrite(b);
    buffer.brelse(b);
}

void initBitmap(Buffer &buffer, superblock &sb)
{

    // 初始化：将bitmap所有位初始为0
    for (int i = 0; i < sb.nbitmap; i++)
    {
        bzero(buffer, sb.bmapstart + i);
    }

    uint nmeta = sb.size - sb.nblocks;
    // 将meta块对应的位设置0
    int n = nmeta / (BSIZE * 8) + 1;

    int i = 0;
    // 将整块设为1
    for (; i < n - 1; i++)
    {
        buf &b = buffer.bread(sb.bmapstart + i);
        set_bits_to_one(b.data, 0, BSIZE * 8);
        buffer.bwrite(b);
        buffer.brelse(b);
    }
    // 处理不足一块的部分
    // 需要处理的bit数
    {
        uint t = nmeta % (BSIZE * 8);
        buf &b = buffer.bread(sb.bmapstart + i);
        set_bits_to_one(b.data, 0, t);
        buffer.bwrite(b);
        buffer.brelse(b);
    }
}

void initDisk(Buffer &buffer, superblock &sb)
{

    // write superblock
    buf &b_sb = buffer.bread(1);
    std::memmove(b_sb.data, &sb, sizeof(sb));
    buffer.bwrite(b_sb);
    buffer.brelse(b_sb);

    // init log
    bzero(buffer, sb.logstart);

    // inode

    // bitmap
    initBitmap(buffer, sb);

    // 将剩余块都写为0
    for (int i = sb.bmapstart + sb.nbitmap; i < sb.size; i++)
    {
        bzero(buffer, i);
    }
}

int main()
{
    uint nmeta = 2 + LOGSIZE + NINODES;
    uint nbitmap = (DSIZE/BSIZE - nmeta) / (8 * BSIZE) + 1;
    superblock sb(FSMAGIC, DSIZE / BSIZE, DSIZE/BSIZE - nmeta - nbitmap,
                  NINODES, LOGSIZE, nbitmap,
                  2, 2 + LOGSIZE,
                  2 + LOGSIZE + NINODES);

    Buffer buffer;

    initDisk(buffer, sb);
}