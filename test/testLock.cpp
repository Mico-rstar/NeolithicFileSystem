#include <cassert>
#include <cstring>
#include <iostream>

#include "../src/sleeplock.h"
#include "../src/spinlock.h"

// 测试Spinlock
void testSpinlock()
{
    Spinlock lock;
    std::cout<< "Spinlock"<<std::endl;
    lock.acquire();
    std::cout<< "In critical section"<<std::endl;
    lock.release();
    std::cout<< "Lock has been released"<<std::endl;
}


// 测试Sleeplock
void testSleeplock()
{
    Spinlock lock;
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
    testSpinlock();
    return 0;
}