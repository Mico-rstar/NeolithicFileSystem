#include <cstring>

#include "log.h"
#include "dbg.h"

void Logger::writeTrans()
{
    if (!log.lock.holding())
        dbg::panic("Logger::writeTrans: lock");
    for (uint i = 0; i < log.lh.cnt; i++)
    {
        buf &from = bf.bread(log.lh.block[i]);
        buf &to = bf.bread(log.start + i + 1);
        std::memmove(&to, &from, BSIZE);

        // -----确保blockno不改变-----
        to.blockno = log.start + i + 1;

        bf.bwrite(to);
        bf.brelse(from);
        bf.brelse(to);
    }
}

void Logger::writeHead()
{
    if (!log.lock.holding())
        dbg::panic("Logger::writeTrans: lock");

    buf &b = bf.bread(log.start);
    LogHeader *hb = (LogHeader *)(b.data);
    hb->cnt = log.lh.cnt;
    for (uint i = 0; i < log.lh.cnt; i++)
    {
        hb->block[i] = log.lh.block[i];
    }
    bf.bwrite(b);
    bf.brelse(b);
}

void Logger::readHead()
{
    if (!log.lock.holding())
        dbg::panic("Logger::readTrans: lock");

    buf &b = bf.bread(log.start);
    LogHeader *hb = (LogHeader *)(b.data);
    log.lh.cnt = hb->cnt;
    for (uint i = 0; i < log.lh.cnt; i++)
    {
        hb->block[i] = log.lh.block[i];
    }
    bf.brelse(b);
}

// 从日志中读入数据，写道目标位置
void Logger::installTrans(int recovering)
{
    for (uint i = 0; i < log.lh.cnt; i++)
    {
        buf &from = bf.bread(log.start + i + 1);
        buf &to = bf.bread(log.lh.block[i]);

        std::memmove(&to, &from, BSIZE);

        // -----确保blockno不改变-----
        to.blockno = log.lh.block[i];

        bf.bwrite(to);
        if (recovering == 0)
            bf.bunpin(to);

        bf.brelse(to);
        bf.brelse(from);
    }
}

void Logger::commit()
{
    // ------ commit ------
    // 1. 将更改的块写入log in disk
    writeTrans(); // Write modified blocks from cache to log

    // 2. 将log结构体写入log in disk
    writeHead(); // Write header to disk -- the real commit

    // 3. 将log中的数据写入到目标位置
    installTrans(0); // Now install writes to home locations

    // 4. 清空log：将空log结构体写入log in disk
    log.lh.cnt = 0;
    writeHead(); // Erase the transaction from the log
}

void Logger::recover()
{
    // ------ recover ------

    // 1. 读取log结构体from disk
    readHead(); // Write header to disk -- the real commit

    // 2. 将log中的数据写入到目标位置
    installTrans(1); // Now install writes to home locations

    // 3. 清空log：将空log结构体写入log in disk
    log.lh.cnt = 0;
    writeHead(); // Erase the transaction from the log
}

Logger::Logger(superblock &sb) : log(sb.logstart, sb.nlog), bf()
{
    log.lock.acquire();
    // TODO: recover from log
    recover();
    log.lock.release();
}

void Logger::beginOP()
{
    log.lock.acquire();

    log.outstanding++;

    log.lock.release();
}

// 不实际写入disk，只把受到更改的块记录到log
void Logger::write(buf &b)
{
    if (!b.lock.holding())
        dbg::panic("Logger.write: lock");

    this->log.lock.acquire();

    if (log.lh.cnt >= log.size - 1)
        dbg::panic("Logger.write: too much operation");

    int i = 0;
    for (; i < log.lh.cnt; i++)
    {
        if (log.lh.block[i] == b.blockno)
            break;
    }
    // block中没有这个块，要加入
    if (i == log.lh.cnt)
    {
        log.lh.block[log.lh.cnt] = b.blockno;
        log.lh.cnt++;
        bf.bpin(b);
    }

    this->log.lock.release();
}

buf &Logger::read(uint blockno)
{
    return bf.bread(blockno);
}

void Logger::relse(buf &b)
{
    bf.brelse(b);
}

// 为了简化实现，这里持有锁进行commit
// 但持有锁commit会导致其他线程进入阻塞，多线程性能极差
void Logger::endOP()
{
    log.lock.acquire();

    log.outstanding--;
    if (log.outstanding == 0)
    {
        commit();
    }

    log.lock.release();
}
