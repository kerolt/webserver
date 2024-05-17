#pragma once

#include "Packet.h"
#include "HttpConn.h"
#include "reactor/EventLoop.h"
#include "thread_pool/ThreadPoolEventLoop.h"
#include "log/Logging.h"

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

typedef std::shared_ptr<SSL_CTX> SP_SSL_CTX;

class Server {
private:
    std::shared_ptr<EventLoop> loop_;
    std::shared_ptr<Channel> channel_;
    UP_ThreadpoolEventLoop thread_pool_;
    int listen_fd_;
    SP_SSL_CTX ctx;
    std::unordered_map<int, std::shared_ptr<HttpConn>> http_map_;
    void HandleConn();
    void HandleClose(std::weak_ptr<Channel> channel);
    void DeleteMap(std::shared_ptr<Channel> channel);
    void ssl_hand_shake(std::weak_ptr<Channel> channel);

public:
    Server(const char* port, int threadnum);
    ~Server();
    void Run();
};
