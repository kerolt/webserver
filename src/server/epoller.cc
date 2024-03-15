#include "epoller.h"
#include <assert.h>
#include <unistd.h>
#include <sys/epoll.h>

Epoller::Epoller(int max_events)
    : epoll_fd_(epoll_create(512))
    , events_(max_events) {
    assert(epoll_fd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epoll_fd_);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool Epoller::DelFd(int fd) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev) == 0;
}

int Epoller::Wait(int timeout_ms) {
    return epoll_wait(epoll_fd_, &events_[0], events_.size(), timeout_ms);
}

int Epoller::GetEventFd(std::size_t i) const {
    assert(i >= 0 && i < events_.size());
    return events_[i].data.fd;
}

uint32_t Epoller::GetEvents(std::size_t i) const {
    assert(i >= 0 && i < events_.size());
    return events_[i].events;
}