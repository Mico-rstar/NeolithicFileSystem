
#include "buffer.h"

class Logger
{
private:
    struct LogHeader
    {
        uint cnt;
        uint block[LOGSIZE];

        LogHeader() : cnt(0) {};
    };

    struct Log
    {
        SleepLock lock;
        uint start;       // log start in disk
        uint size;        // log size in disk
        uint outstanding; // how many FS sys calls are executing.
        LogHeader lh;

        Log() = default;
        Log(uint _start, uint _size) : lock(), start(_start), size(_size), outstanding(0), lh() {};
    } log;

    Buffer bf;

    void writeTrans();
    void writeHead();
    void readHead();
    void installTrans(int recovering);
    void commit();
    void recover();

public:
    friend class DiskInit;
    Logger(superblock &sb);
    void beginOP();
    void write(buf &buf);
    buf &read(uint blockno);
    void relse(buf &b);
    void endOP();
};