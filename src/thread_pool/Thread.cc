#include "Thread.h"

Thread::Thread(const ThreadFunc&& Func, const std::string& n)
    : started_(false)
    , joined_(false)
    , pthreadId(0)
    , func(Func)
    , name_(n) {
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_)
        pthread_detach(pthreadId);
}

void Thread::setDefaultName() {
    if (name_.empty())
        name_ += "Thread";
}

void* startThread(void* obj) {
    auto* data = (ThreadData*) obj;
    data->runInThread();
    DeleteElement<ThreadData>(data);
    return nullptr;
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    auto* data = NewElement<ThreadData>(func, name_);
    if (pthread_create(&pthreadId, nullptr, &startThread, data)) {
        started_ = false;
        DeleteElement<ThreadData>(data);
    }
}

int Thread::join() {
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId, nullptr);
}

bool Thread::started() const {
    return started_;
}

const std::string& Thread::name() const {
    return name_;
}
