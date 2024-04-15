#ifndef HTTP_CONN_H_
#define HTTP_CONN_H_

#include <arpa/inet.h>
#include <bits/types/struct_iovec.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <atomic>

#include "buffer.h"
#include "http_response.h"
#include "http_request.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();
    void Init(int sock_fd, const struct sockaddr_in& addr);
    void Close();
    ssize_t Read(int* err);
    ssize_t Write(int* err);
    bool Process();

    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    struct sockaddr_in GetAddr() const;
    bool IsKeepAlive() const;
    int ToWriteBytes();

public:
    static bool IsET;
    static const char* SrcDir;
    static std::atomic<int> UserCnt;

private:
    int fd_;
    bool is_close_;
    struct sockaddr_in addr_;

    int iov_cnt_;
    struct iovec iov_[2];

    Buffer write_buf_;
    Buffer read_buf_;
    HttpResponse response_;
    HttpRequest request_;
};

#endif /* HTTP_CONN_H_ */
