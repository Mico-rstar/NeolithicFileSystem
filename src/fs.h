#pragma once
#include <string>

#define ROOTINO  1   // root i-number

#define NFILE 30

#define BSIZE 1024                // block size
#define DSIZE (8 * 1024 * 1024)   // disk size
#define MAXOPBLOCKS 10            // max # of blocks any FS op writes
#define LOGSIZE (MAXOPBLOCKS * 3) // max data blocks in on-disk log
#define NBUF (MAXOPBLOCKS * 3)    // size of disk block cache
#define NINODES 200
#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define NBB (8 * BSIZE) // number of bits per block
#define MAXFILE (NDIRECT + NINDIRECT)
#define MAXPATH      128   // maximum file path name

// Disk layout:
// [ boot block | super block | log | inode blocks | free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock
{
    uint magic;      // Must be FSMAGIC
    uint size;       // Size of file system image (blocks)
    uint nblocks;    // Number of data blocks
    uint ninodes;    // Number of inode blocks.
    uint nlog;       // Number of log blocks
    uint nbitmap;    // Number of bitmap blocks
    uint logstart;   // Block number of first log block
    uint inodestart; // Block number of first inode block
    uint bmapstart;  // Block number of first free map block

    superblock() = default;
    superblock(uint _magic, uint _size,
               uint _nblocks, uint _ninodes,
               uint _nlog, uint _nbitmap,
               uint _logstart,
               uint _inodestart, uint _bmapstart) : magic(_magic), size(_size),
                                                    nblocks(_nblocks), ninodes(_ninodes),
                                                    nlog(_nlog), logstart(_logstart),
                                                    nbitmap(_nbitmap),
                                                    inodestart(_inodestart), bmapstart(_bmapstart) {}
};

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent
{
    ushort inum;
    char name[DIRSIZ];
};

#define FSMAGIC 0x10203040

const char IMGPATH[] = "./img/fs.img";


struct fsstat
{
    uint nfb; // number of free block
};
