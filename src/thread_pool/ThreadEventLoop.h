#pragma once

#include <vector>

#include "Thread.h"
#include "../reactor/EventLoop.h"

class ThreadEventLoop {
public:
    ThreadEventLoop();
    ~ThreadEventLoop();
    void start();
    std::shared_ptr<EventLoop> getLoop();

private:
    void Loop();

    std::shared_ptr<EventLoop> loop_;
    std::unique_ptr<Thread, decltype(DeleteElement<Thread>)*> thread_;
};
