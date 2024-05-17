#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "Channel.h"
#include "../Packet.h"
#include "../HttpConn.h"
#include "../memory_pool/MemoryPool.h"

#define MAXFDS 10000
#define EVENTS 4096
#define EPOLLWAIT_TIME -1

class Epoll {
private:
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
    std::unordered_map<int, std::shared_ptr<Channel>> channel_map_;

public:
    Epoll();
    ~Epoll();
    void Add(const std::shared_ptr<Channel>& request);
    void Update(const std::shared_ptr<Channel>& request);
    void Del(const std::shared_ptr<Channel>& request);
    void Poll(std::vector<std::shared_ptr<Channel>>& req);
};
