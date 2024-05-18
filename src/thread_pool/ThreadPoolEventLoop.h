#pragma once

#include "ThreadEventLoop.h"
#include "../memory_pool/MemoryPool.h"

class ThreadPoolEventLoop {
public:
    explicit ThreadPoolEventLoop(int thread_num);
    ~ThreadPoolEventLoop();
    void start();
    std::shared_ptr<EventLoop> GetNextLoop();

private:
    std::vector<std::shared_ptr<ThreadEventLoop>> threads_loop_;
    int thread_num_;
    int index_;
};

typedef std::unique_ptr<ThreadPoolEventLoop, decltype(DeleteElement<ThreadPoolEventLoop>)*> UP_ThreadpoolEventLoop;
