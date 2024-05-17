#include "Server.h"

Server::Server(const char* port, int threadnum)
    : loop_(NewElement<EventLoop>(), DeleteElement<EventLoop>)
    , channel_(NewElement<Channel>(loop_), DeleteElement<Channel>)
    , thread_pool_(NewElement<ThreadPoolEventLoop>(threadnum), DeleteElement<ThreadPoolEventLoop>) {
    // ssl初始化
    if (GetConf().GetSSL()) {
        SSL_load_error_strings();
        SSL_library_init();
        ctx = SP_SSL_CTX(SSL_CTX_new(SSLv23_method()), SSL_CTX_free);
        int r = SSL_CTX_use_certificate_file(ctx.get(), GetConf().GetSSLCrtPath().c_str(), SSL_FILETYPE_PEM);
        if (r <= 0)
            LOG_ERROR << "ssl_ctx_use_certificate_file failed";
        r = SSL_CTX_use_PrivateKey_file(ctx.get(), GetConf().GetSSLKeyPath().c_str(), SSL_FILETYPE_PEM);
        if (r <= 0)
            LOG_ERROR << "ssl_ctx_user_privatekey_file failed";
        r = SSL_CTX_check_private_key(ctx.get());
        if (!r)
            LOG_ERROR << "ssl_ctx_check_private_key failed";
    }
    listen_fd_ = TcpListen(NULL, port, NULL);
    SetNonBlock(listen_fd_);
    channel_->SetFd(listen_fd_);
}

Server::~Server() {
    Close(listen_fd_);
}

void Server::ssl_hand_shake(std::weak_ptr<Channel> wpchannel) {
    std::shared_ptr<Channel> channel = wpchannel.lock();
    int result = SSL_do_handshake(channel->GetSsl().get());
    int connfd = channel->GetFd();
    if (1 == result) {
        channel->SetSslConnect(true);
        channel->SetRetEvents(EPOLLIN | EPOLLET);
        std::shared_ptr<HttpConn> connhttp(NewElement<HttpConn>(channel), DeleteElement<HttpConn>);
        http_map_[connfd] = move(connhttp);
        channel->GetLoop().lock()->UpdatePoller(channel);
        return;
    }
    int error = SSL_get_error(channel->GetSsl().get(), result);
    if (SSL_ERROR_WANT_WRITE == error) {
        channel->SetRetEvents(EPOLLOUT);
        channel->GetLoop().lock()->UpdatePoller(channel);
    } else if (SSL_ERROR_WANT_READ == error) {
        channel->SetRetEvents(EPOLLIN);
        channel->GetLoop().lock()->UpdatePoller(channel);
    } else {
        LOG_ERROR << "SSL handshake error";
        channel->SetDeleted(true);
        channel->GetLoop().lock()->AddTimer(channel, 0);
    }
}

void Server::HandleConn() {
    struct sockaddr_storage cliaddr {};
    socklen_t clilen = sizeof(cliaddr);
    int conn_fd;
    while ((conn_fd = Accept(listen_fd_, (SA*) &cliaddr, &clilen)) >= 0) {
        LOG_INFO << "Accept fd = " << conn_fd;
        SetNonBlock(conn_fd);
        std::shared_ptr<EventLoop> next_loop = thread_pool_->GetNextLoop();
        std::shared_ptr<Channel> conn_channel(NewElement<Channel>(next_loop), DeleteElement<Channel>);
        conn_channel->SetFd(conn_fd);
        std::weak_ptr<Channel> wp_channel = conn_channel;
        conn_channel->SetCloseHandler(bind(&Server::HandleClose, this, wp_channel));
        if (GetConf().GetSSL()) {
            conn_channel->SetRetEvents(EPOLLIN);
            std::shared_ptr<SSL> ssl(SSL_new(ctx.get()), SSL_free);
            SSL_set_fd(ssl.get(), conn_fd);
            SSL_set_accept_state(ssl.get());
            conn_channel->SetSsl(ssl);
            conn_channel->SetSslConnect(false);
            conn_channel->SetReadHandler(bind(&Server::ssl_hand_shake, this, wp_channel));
        } else {
            conn_channel->SetRetEvents(EPOLLIN | EPOLLET);
            std::shared_ptr<HttpConn> connhttp(NewElement<HttpConn>(conn_channel), DeleteElement<HttpConn>);
            http_map_[conn_fd] = std::move(connhttp);
        }
        next_loop->QueueInLoop(bind(&EventLoop::AddPoller, next_loop, std::move(conn_channel)));
    }
}

void Server::Run() {
    thread_pool_->start();
    channel_->SetRetEvents(EPOLLIN | EPOLLET);
    channel_->SetReadHandler(std::bind(&Server::HandleConn, this));
    loop_->AddPoller(channel_);
    LOG_INFO << "Server running...";
    loop_->Loop();
}

void Server::HandleClose(std::weak_ptr<Channel> channel) {
    std::shared_ptr<Channel> spchannel = channel.lock();
    loop_->QueueInLoop(bind(&Server::DeleteMap, this, spchannel));
    spchannel->GetLoop().lock()->RemovePoller(spchannel);
}

void Server::DeleteMap(std::shared_ptr<Channel> channel) {
    http_map_.erase(channel->GetFd());
}
