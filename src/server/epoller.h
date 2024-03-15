#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <cstddef>
#include <cstdint>
#include <sys/epoll.h>
#include <vector>

class Epoller {
public:
    explicit Epoller(int max_events = 1024);

    ~Epoller();

    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeout_ms = -1);
    int GetEventFd(std::size_t i) const;
    uint32_t GetEvents(std::size_t i) const;

private:
    int epoll_fd_;
    std::vector<epoll_event> events_;
};

#endif /* EPOLLER_H_ */
