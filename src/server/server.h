#ifndef SERVER_H_
#define SERVER_H_

#include <cstdint>
#include <arpa/inet.h>

#include <memory>
#include <unordered_map>

#include "epoller.h"
#include "thread_pool.h"
#include "timer_heap.h"
#include "http_conn.h"
#include "config.h"

class WebServer {
public:
    WebServer(Config config);
    ~WebServer();
    void Run();

private:
    bool InitSocket();
    void InitEventMode(int trig_mode);
    void AddClient(int fd, struct sockaddr_in addr);

    void DealListen();
    void DealWrite(HttpConn* client);
    void DealRead(HttpConn* client);

    void OnRead(HttpConn* client);
    void OnWrite(HttpConn* client);
    void OnProcess(HttpConn* client);

    void SendError(int fd, const char* msg);
    void ExtentTime(HttpConn* client);
    void CloseConn(HttpConn* client);
    int SetFdNonBlock(int fd);

private:
    const int kMaxFd = 65536;

    Config config_;
    int port_;
    int timeout_ms_;
    int listen_fd_;
    bool open_linger_;

    bool is_close_;
    char* src_dir_;

    uint32_t listen_evt_;
    uint32_t conn_evt_;

    std::unique_ptr<TimerHeap> timer_;
    std::unique_ptr<Epoller> epoller_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unordered_map<int, HttpConn> users_;
};

#endif /* SERVER_H_ */
