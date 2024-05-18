#include "Epoll.h"

Epoll::Epoll()
    : events_(EVENTS) {
    epoll_fd_ = EpollCreate(MAXFDS);
}

Epoll::~Epoll() {
    Close(epoll_fd_);
}

void Epoll::Add(const std::shared_ptr<Channel>& request) {
    int fd = request->GetFd();
    SE ev;
    ev.events = request->GetRetEvents();
    ev.data.fd = fd;
    EpollCtl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    channel_map_[fd] = request;
}

void Epoll::Update(const std::shared_ptr<Channel>& request) {
    int fd = request->GetFd();
    SE ev;
    ev.events = request->GetRetEvents();
    ev.data.fd = fd;
    EpollCtl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

void Epoll::Del(const std::shared_ptr<Channel>& request) {
    int fd = request->GetFd();
    SE ev;
    ev.events = request->GetRetEvents();
    ev.data.fd = fd;
    EpollCtl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev);
    channel_map_.erase(fd);
}

void Epoll::Poll(std::vector<std::shared_ptr<Channel>>& req) {
    int n = epoll_wait(epoll_fd_, &*events_.begin(), EVENTS, EPOLLWAIT_TIME);
    for (int i = 0; i < n; ++i) {
        int fd = events_[i].data.fd;
        std::shared_ptr<Channel> temp = channel_map_[fd];
        temp->SetEvents(events_[i].events);
        req.emplace_back(std::move(temp));
    }
}
