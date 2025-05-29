#include <cassert>
#include <cstring>
#include <iostream>

#include "../src/sleeplock.h"




// 测试Sleeplock
void testSleeplock()
{
    SleepLock lock;
    std::cout<< "Sleeplock"<<std::endl;
    lock.acquire();
    std::cout<< "In critical section"<<std::endl;
    lock.release();
    std::cout<< "Lock has been released"<<std::endl;
}




int main()
{
    std::cout << "=== Testing Lock ===" << std::endl;
    testSleeplock();
    return 0;
}