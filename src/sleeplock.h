#pragma once

#include <mutex>

class SleepLock
{
private:
    std::mutex mtx;
    bool locked = false;

public:
    void acquire()
    {
        mtx.lock();
        locked = true;
    }

    void release()
    {
        mtx.unlock();
        locked = false;
    }

    bool holding() const
    {
        return locked;
    }
};