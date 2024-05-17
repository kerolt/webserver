#include "ThreadEventLoop.h"

ThreadEventLoop::ThreadEventLoop()
    : loop_(NewElement<EventLoop>(), DeleteElement<EventLoop>)
    , thread_(NewElement<Thread>(std::bind(&ThreadEventLoop::Loop, this)), DeleteElement<Thread>) {
}

ThreadEventLoop::~ThreadEventLoop() = default;

void ThreadEventLoop::Loop() {
    loop_->Loop();
}

void ThreadEventLoop::start() {
    thread_->start();
}

std::shared_ptr<EventLoop> ThreadEventLoop::getLoop() {
    return loop_;
}
