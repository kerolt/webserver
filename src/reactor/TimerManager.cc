#include "TimerManager.h"

typedef struct timeval ST;

TimerNode::TimerNode(std::shared_ptr<Channel> channel, int timeout)
    : channel_(channel) {
    ST now;
    gettimeofday(&now, nullptr);
    expired_time_ = (Long) now.tv_sec * 1000 + now.tv_usec / 1000 + timeout;
}

TimerNode::~TimerNode() {
}

Long TimerNode::GetExpiredTime() {
    return expired_time_;
}

void TimerNode::Update(int timeout) {
    ST now;
    gettimeofday(&now, nullptr);
    expired_time_ = (Long) now.tv_sec * 1000 + now.tv_usec / 1000 + timeout;
}

bool TimerNode::IsValid() {
    ST now;
    gettimeofday(&now, nullptr);
    Long temp = (Long) now.tv_sec * 1000 + now.tv_usec / 1000;
    if (temp < expired_time_)
        return true;
    return false;
}

bool TimerNode::IsDeleted() {
    return channel_->IsDeleted();
}

std::shared_ptr<Channel> TimerNode::GetChannel() {
    return channel_;
}

void TimerManager::AddTimer(const std::shared_ptr<Channel>& channel, int timeout) {
    // std::shared_ptr<TimerNode> timer_node(new TimerNode(channel_,timeout));
    std::shared_ptr<TimerNode> timer_node(NewElement<TimerNode>(channel, timeout), DeleteElement<TimerNode>);
    int cfd = channel->GetFd();
    // if(timer_map_.find(cfd)==timer_map_.end())
    if (channel->IsFirst()) {
        timer_heap_.push(timer_node);
        channel->SetNotFirst();
    }
    timer_map_[cfd] = std::move(timer_node);
}

void TimerManager::HandleExpiredEvent() {
    while (!timer_heap_.empty()) {
        std::shared_ptr<TimerNode> now = timer_heap_.top();
        if (now->IsDeleted() || !now->IsValid()) {
            timer_heap_.pop();
            std::shared_ptr<TimerNode> timer_node = timer_map_[now->GetChannel()->GetFd()];
            if (now == timer_node) {
                timer_map_.erase(now->GetChannel()->GetFd());
                now->GetChannel()->HandleClose();
            } else
                timer_heap_.push(timer_node);
        } else
            break;
    }
}
