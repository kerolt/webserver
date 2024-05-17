#pragma once

#include <sys/epoll.h>

#include <functional>
#include <memory>
#include "EventLoop.h"
#include "../log/Logging.h"
#include "../memory_pool/MemoryPool.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

class EventLoop;

class Channel {
    using CallBack = std::function<void()>;

public:
    explicit Channel(const std::shared_ptr<EventLoop>& loop);
    ~Channel();

    void SetReadHandler(CallBack&& readHandler);
    void SetWriteHandler(CallBack&& writeHandler);
    void SetCloseHandler(CallBack&& closeHandler);
    void SetDeleted(bool Deleted);
    void HandleEvent();

    std::weak_ptr<EventLoop> GetLoop();
    void HandleClose();
    void SetFd(int fd);
    void SetRetEvents(int ret_events);
    void SetEvents(int events);
    void SetNotFirst();
    bool IsFirst();
    int GetFd();
    int GetRetEvents();
    bool IsDeleted();

    bool GetSslConnect();
    void SetSsl(std::shared_ptr<SSL> SSL);
    void SetSslConnect(bool ssl_connect);
    std::shared_ptr<SSL> GetSsl();

private:
    int fd_{};
    int events_{};
    int ret_events_{};
    bool deleted_;
    bool is_first_;
    bool ssl_connect_{};

    std::shared_ptr<SSL> ssl_;
    std::weak_ptr<EventLoop> loop_;

    CallBack read_handler_;
    CallBack write_handler_;
    CallBack close_handler_;
};
