#pragma once

#include "inode.h"

// namespace fspermissions
// {
//     char readable = 1 << 0;
//     char writable = 1 << 1;
// }

struct file
{

    enum
    {
        FD_NONE,
        FD_DIR,
        FD_FILE
    } type;

    int ref; // reference count
    char mask;
    inode *ip;
    bool readable;
    bool writable;

    uint off;
};

struct Ftable
{
    SleepLock lock;
    struct file file[NFILE];
};

class File
{
private:
    Ftable ftable;
    Inode in;

    inode *create(char *path, short type);

    int isdirempty(struct inode *dp);
 
public:
    file *open(char *path, bool readable, bool writable);
    void close(file *f);
    int read(file *f, char *buf, uint n);

    int write(file *f, char *buf, uint n);
    // int seek(file &f, uint offset);
    // int unlink(char *path);
    int unlink(char *path);

    // dir
    int mkdir(char *path);

    // int rmdir(char *path);
    void tree(char *path);
};