#pragma once

#include <sys/time.h>

#include <queue>
#include <vector>
#include <unordered_map>

#include "Channel.h"
#include "../memory_pool/MemoryPool.h"

using Long = long long;

class Channel;

class TimerNode {
public:
    TimerNode(std::shared_ptr<Channel> channel, int timeout);
    ~TimerNode();
    Long GetExpiredTime();
    void Update(int timeout);
    bool IsValid();
    bool IsDeleted();
    std::shared_ptr<Channel> GetChannel();

private:
    std::shared_ptr<Channel> channel_;
    Long expired_time_;
};

class TimerManager {
public:
    void AddTimer(const std::shared_ptr<Channel>& channel, int timeout);
    void HandleExpiredEvent();

private:
    struct TimerCmp {
        bool operator()(std::shared_ptr<TimerNode>& a, std::shared_ptr<TimerNode>& b) const {
            return a->GetExpiredTime() > b->GetExpiredTime();
        }
    };

    std::priority_queue<std::shared_ptr<TimerNode>, std::vector<std::shared_ptr<TimerNode>>, TimerCmp> timer_heap_;
    std::unordered_map<int, std::shared_ptr<TimerNode>> timer_map_;
};
