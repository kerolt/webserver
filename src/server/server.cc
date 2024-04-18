#include "server.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <memory>
#include <utility>
#include <functional>

#include "log.h"
#include "config.h"
#include "epoller.h"
#include "sql_pool.h"
#include "http_conn.h"
#include "thread_pool.h"
#include "timer_heap.h"

WebServer::WebServer(Config config)
    : config_(config)
    , is_close_(false)
    , timer_(std::make_unique<TimerHeap>())
    , epoller_(std::make_unique<Epoller>())
    , thread_pool_(std::make_unique<ThreadPool>(config_.thread_num))
    , timeout_ms_(config_.timeout_ms)
    , port_(config_.port)
    , open_linger_(config_.opt_linger) {
    // 成员初始化
    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    strncat(src_dir_, "/web/", 16);
    HttpConn::UserCnt = 0;
    HttpConn::SrcDir = src_dir_;
    SqlConnPool::Instance()->Init("localhost", config_.port, config_.sql_user.c_str(), config_.sql_pwd.c_str(), config_.db_name.c_str());

    // Epoll触发模式
    InitEventMode(config_.trig_mode);
    if (!InitSocket()) {
        is_close_ = true;
    }

    // 设置日志
    if (config_.open_log) {
        Log::Instance()->Init("./log", ".log", config_.log_que_size, config_.log_level);
        if (is_close_) {
            LOG_ERROR("!!! Server init error");
        } else {
            LOG_INFO("Server init success :)");
            LOG_INFO("Port:%d, OpenLinger: %s", port_,
                     config_.opt_linger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                     (listen_evt_ & EPOLLET ? "ET" : "LT"),
                     (conn_evt_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("Log level: %d", config_.log_level);
            LOG_INFO("Web root: %s", HttpConn::SrcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", config_.conn_pool_num, config_.thread_num);
        }
    }
}

WebServer::~WebServer() {
    close(listen_fd_);
    is_close_ = true;
    free(src_dir_);
    SqlConnPool::Instance()->Close();
}

void WebServer::Run() {
    int time_ms = -1;
    if (!is_close_) {
        LOG_INFO("Server run... :)");
    }

    // 服务器处理事件
    while (!is_close_) {
        if (timeout_ms_ > 0) {
            time_ms = timer_->GetNextTick();
        }
        int evt_cnt = epoller_->Wait(time_ms);
        for (int i = 0; i < evt_cnt; i++) {
            int fd = epoller_->GetEventFd(i);
            uint32_t event = epoller_->GetEvents(i);
            if (fd == listen_fd_) {
                DealListen();
            } else if (event & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
            } else if (event & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            } else if (event & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event :(");
            }
        }
    }
}

bool WebServer::InitSocket() {
    if (port_ > 65536 || port_ < 1024) {
        LOG_ERROR("Port %d error!!!", port_);
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    struct linger lgr = {0};
    if (open_linger_) {
        lgr.l_onoff = 1;
        lgr.l_linger = 1;
    }

    // 创建监听socket
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        LOG_ERROR("Can't create socket!!!");
        return false;
    }

    // 设置linger
    int ret = setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &lgr, sizeof(lgr));
    if (ret < 0) {
        LOG_ERROR("Init linger error!!!");
        close(listen_fd_);
        return false;
    }

    // 设置端口复用
    int optval = 1;
    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval, sizeof(int));
    if (ret < 0) {
        LOG_ERROR("Use setsocketopt error!!!");
        close(listen_fd_);
        return false;
    }

    ret = bind(listen_fd_, (struct sockaddr*) &addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERROR("Bind port: %d error!!!", port_);
        close(listen_fd_);
        return false;
    }

    ret = listen(listen_fd_, 6);
    if (ret < 0) {
        LOG_ERROR("Listen port: %d error!!!", port_);
        close(listen_fd_);
        return false;
    }

    ret = epoller_->AddFd(listen_fd_, listen_evt_ | EPOLLIN);
    if (ret == 0) {
        LOG_ERROR("Add listen error!!!");
        close(listen_fd_);
        return false;
    }

    SetFdNonBlock(listen_fd_);
    LOG_INFO("Server port: %d", port_);
    return true;
}

void WebServer::InitEventMode(int trig_mode) {
    listen_evt_ = EPOLLRDHUP;
    conn_evt_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (config_.trig_mode) {
        case 0:
            break;
        case 1:
            conn_evt_ |= EPOLLET;
            break;
        case 2:
            listen_evt_ |= EPOLLET;
            break;
        case 3:
            listen_evt_ |= EPOLLET;
            conn_evt_ |= EPOLLET;
            break;
        default:
            listen_evt_ |= EPOLLET;
            conn_evt_ |= EPOLLET;
            break;
    }
    HttpConn::IsET = conn_evt_ & EPOLLET;
}

void WebServer::AddClient(int fd, struct sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].Init(fd, addr);
    if (timeout_ms_ > 0) {
        timer_->Add(fd, timeout_ms_, std::bind(&WebServer::CloseConn, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | conn_evt_);
    SetFdNonBlock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

void WebServer::DealListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listen_fd_, (struct sockaddr*) &addr, &len);
        if (fd <= 0) {
            return;
        } else if (HttpConn::UserCnt >= kMaxFd) {
            SendError(fd, "Server busy now!!!");
            LOG_WARN("Clients is full!!!");
            return;
        }
        AddClient(fd, addr);
    } while (listen_evt_ & EPOLLET);
}

void WebServer::DealWrite(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::DealRead(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    thread_pool_->AddTask(std::bind(&WebServer::OnRead, this, client));
}

void WebServer::OnRead(HttpConn* client) {
    assert(client);
    int ret = -1;
    int read_errno = 0;
    ret = client->Read(&read_errno);
    if (ret <= 0 && read_errno != EAGAIN) {
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite(HttpConn* client) {
    assert(client);
    int ret = -1;
    int write_errno = 0;
    ret = client->Write(&write_errno);
    if (client->ToWriteBytes() == 0) {
        // 传输完成
        if (client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    } else if (ret < 0) {
        if (write_errno == EAGAIN) {
            // 继续传输
            epoller_->ModFd(client->GetFd(), conn_evt_ | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}

void WebServer::OnProcess(HttpConn* client) {
    if (client->Process()) {
        epoller_->ModFd(client->GetFd(), conn_evt_ | EPOLLOUT);
    } else {
        epoller_->ModFd(client->GetFd(), conn_evt_ | EPOLLIN);
    }
}

void WebServer::SendError(int fd, const char* msg) {
    assert(fd > 0);
    if (send(fd, msg, strlen(msg), 0) < 0) {
        LOG_ERROR("Send error message to client[%d] error!!!", fd);
    }
    close(fd);
}

void WebServer::ExtentTime(HttpConn* client) {
    assert(client);
    if (timeout_ms_ > 0) {
        timer_->Adjust(client->GetFd(), timeout_ms_);
    }
}

void WebServer::CloseConn(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

int WebServer::SetFdNonBlock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
