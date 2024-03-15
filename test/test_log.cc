#include "log.h"
#include <iostream>
#include <unistd.h>
#include <thread>

void Thread1() {
    for (int i = 0; i < 10; i++) {
        LOG_DEBUG("thread1");
        LOG_INFO("thread1");
    }
}

void Thread2() {
    for (int i = 0; i < 10; i++) {
        LOG_WARN("thread2");
        LOG_ERROR("thread2");
    }
}

int main() {
    // 初始化日志系统
    Log::Instance()->Init("./", ".log", 1024, DEBUG);

    // 记录不同级别的日志信息
    // LOG_DEBUG("This is a %s message.", "debug");
    // LOG_INFO("This is an %s message.", "info");
    // LOG_WARN("This is a %s message.", "warning");
    // LOG_ERROR("This is an %s message.", "error");
    // sleep(1);
    std::thread t1(Thread1);
    std::thread t2(Thread2);
    t1.join();
    t2.join();
}