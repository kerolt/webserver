#include "Channel.h"

void Channel::SetReadHandler(CallBack&& readHandler) {
    read_handler_ = readHandler;
}

void Channel::SetWriteHandler(CallBack&& writeHandler) {
    write_handler_ = writeHandler;
}

void Channel::SetCloseHandler(CallBack&& closeHandler) {
    close_handler_ = closeHandler;
}

void Channel::SetDeleted(bool Deleted) {
    deleted_ = Deleted;
}

void Channel::HandleEvent() {
    if (events_ & EPOLLIN)
        read_handler_();
    else if (events_ & EPOLLOUT)
        write_handler_();
}

void Channel::HandleClose() {
    close_handler_();
}

Channel::Channel(const std::shared_ptr<EventLoop>& loop)
    : loop_(loop)
    , deleted_(false)
    , is_first_(true) {
}

void Channel::SetNotFirst() {
    is_first_ = false;
}

Channel::~Channel() {
    LOG_DEBUG << "Delete fd = " << fd_;
    Close(fd_);
}
void Channel::SetFd(int fd) {
    fd_ = fd;
}

void Channel::SetRetEvents(int ret_events) {
    ret_events_ = ret_events;
}

void Channel::SetEvents(int events) {
    events_ = events;
}

void Channel::SetSsl(std::shared_ptr<SSL> SSL) {
    ssl_ = SSL;
}

void Channel::SetSslConnect(bool ssl_connect) {
    ssl_connect_ = ssl_connect;
}

int Channel::GetFd() {
    return fd_;
}

int Channel::GetRetEvents() {
    return ret_events_;
}

bool Channel::IsDeleted() {
    return deleted_;
}

bool Channel::IsFirst() {
    return is_first_;
}

std::weak_ptr<EventLoop> Channel::GetLoop() {
    return loop_;
}

std::shared_ptr<SSL> Channel::GetSsl() {
    return ssl_;
}

bool Channel::GetSslConnect() {
    return ssl_connect_;
}
