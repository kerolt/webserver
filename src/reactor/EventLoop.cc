#include "EventLoop.h"

bool EventLoop::quit_ = false;

EventLoop::EventLoop()
    : poller_(NewElement<Epoll>(), DeleteElement<Epoll>)
    , looping_(false)
    , timer_manager_(NewElement<TimerManager>(), DeleteElement<TimerManager>) {
    wakeup_fd_ = Eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
}

void EventLoop::AddPoller(const std::shared_ptr<Channel>& channel) {
    // timer_manager_->AddTimer(channel_,DEFAULT_KEEP_ALIVE_TIME);
    poller_->Add(channel);
}

void EventLoop::UpdatePoller(const std::shared_ptr<Channel>& channel) {
    poller_->Update(channel);
}

void EventLoop::RemovePoller(const std::shared_ptr<Channel>& channel) {
    poller_->Del(channel);
}

void EventLoop::Loop() {
    wakeup_channel_ = std::shared_ptr<Channel>(NewElement<Channel>(shared_from_this()), DeleteElement<Channel>);
    wakeup_channel_->SetFd(wakeup_fd_);
    wakeup_channel_->SetRetEvents(EPOLLIN | EPOLLET);
    wakeup_channel_->SetReadHandler(std::bind(&EventLoop::DoPendingFunctors, shared_from_this()));
    AddPoller(wakeup_channel_);
    std::vector<std::shared_ptr<Channel>> channels;
    while (!quit_) {
        poller_->Poll(channels);
        for (auto& channel : channels) {
            channel->HandleEvent();
        }
        channels.clear();
        timer_manager_->HandleExpiredEvent();
    }
}

void EventLoop::AddTimer(std::shared_ptr<Channel> channel, int timeout) {
    timer_manager_->AddTimer(std::move(channel), timeout);
}

void EventLoop::QueueInLoop(Functor&& cb) {
    {
        MutexLockGuard lock(mutex_);
        pending_functor_.emplace_back(std::move(cb));
    }
    uint64_t buffer = 1;
    if (write(wakeup_fd_, &buffer, sizeof(buffer)) < 0) {
        LOG_ERROR << "Wake up write error";
    }
}

void EventLoop::DoPendingFunctors() {
    uint64_t buffer;
    if (read(wakeup_fd_, &buffer, sizeof(buffer)) < 0)
        LOG_ERROR << "Wake up read error";
    std::vector<Functor> next;
    {
        MutexLockGuard lock(mutex_);
        next.swap(pending_functor_);
    }
    for (auto& func : next) {
        func();
    }
}

void EventLoop::SetQuit(int a) {
    quit_ = true;
}
