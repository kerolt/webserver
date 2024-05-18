#pragma once

#include <queue>

#include "Epoll.h"
#include "TimerManager.h"
#include "../mutex/MutexLock.h"

class Channel;
class Epoll;
class TimerManager;

class EventLoop : public std::enable_shared_from_this<EventLoop> {
    using Functor = std::function<void()>;

public:
    EventLoop();

    void Loop();
    void QueueInLoop(Functor&& cb);

    void AddPoller(const std::shared_ptr<Channel>& channel);
    void UpdatePoller(const std::shared_ptr<Channel>& channel);
    void RemovePoller(const std::shared_ptr<Channel>& channel);
    void AddTimer(std::shared_ptr<Channel> channel, int timeout);

    void DoPendingFunctors();
    static void SetQuit(int);

private:
    static bool quit_;
    int wakeup_fd_;
    bool looping_;
    
    MutexLock mutex_;
    std::vector<Functor> pending_functor_;    // 等待处理的函数
    std::shared_ptr<Epoll> poller_;           // IO多路复用分发器
    std::shared_ptr<Channel> wakeup_channel_; // 用于异步唤醒的channel
    std::shared_ptr<TimerManager> timer_manager_;
};
