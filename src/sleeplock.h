#include <mutex>

class SleepLock
{
private:
    std::mutex mtx;

public:
    void acquire()
    {
        mtx.lock();
    }

    void release()
    {
        mtx.unlock();
    }
};