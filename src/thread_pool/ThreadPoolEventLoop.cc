#include "ThreadPoolEventLoop.h"

ThreadPoolEventLoop::ThreadPoolEventLoop(int thread_num)
    : thread_num_(thread_num)
    , index_(0) {
    threads_loop_.reserve(thread_num_);
    for (int i = 0; i < thread_num_; ++i) {
        std::shared_ptr<ThreadEventLoop> t(NewElement<ThreadEventLoop>(), DeleteElement<ThreadEventLoop>);
        threads_loop_.emplace_back(t);
    }
}

ThreadPoolEventLoop::~ThreadPoolEventLoop() {
    threads_loop_.clear();
}

void ThreadPoolEventLoop::start() {
    for (auto& thread_loop : threads_loop_) {
        thread_loop->start();
    }
}

std::shared_ptr<EventLoop> ThreadPoolEventLoop::GetNextLoop() {
    index_ = (index_ + 1) % thread_num_;
    return threads_loop_[index_]->getLoop();
}
