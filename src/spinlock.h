#pragma once

#include <atomic>
#include <thread>

class Spinlock {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT; // 初始化为 false

public:
    void acquire() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // 自旋等待（可插入 CPU 暂停指令优化）
            #ifdef __x86_64__
                __builtin_ia32_pause(); // x86 的 PAUSE 指令（减少 CPU 功耗）
            #endif
        }
    }

    void release() {
        flag.clear(std::memory_order_release);
    }
};