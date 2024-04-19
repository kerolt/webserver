#include "http_conn.h"

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "log.h"

// init static variable
const char* HttpConn::SrcDir{};
std::atomic<int> HttpConn::UserCnt{};
bool HttpConn::IsET{};

HttpConn::HttpConn() {
    fd_ = -1;
    addr_ = {0};
    is_close_ = true;
}

HttpConn::~HttpConn() {
    Close();
}

void HttpConn::Init(int sock_fd, const struct sockaddr_in& addr) {
    assert(sock_fd > 0);
    UserCnt++;
    addr_ = addr;
    fd_ = sock_fd;
    is_close_ = false;
    read_buf_.RetrieveAll();
    write_buf_.RetrieveAll();
    LOG_INFO("++ Client[%d](%s:%d) come in, user count is %d", fd_, GetIP(), GetPort(), (int) UserCnt);
}

void HttpConn::Close() {
    response_.UnmapFile();
    if (!is_close_) {
        is_close_ = true;
        UserCnt--;
        close(fd_);
        LOG_INFO("-- Client[%d](%s:%d) quit, user count is %d", fd_, GetIP(), GetPort(), (int) UserCnt);
    }
}

ssize_t HttpConn::Read(int* err) {
    ssize_t len = -1;
    do {
        // 将缓冲区中的数据读入fd_
        len = read_buf_.ReadFromFd(fd_, err);
        if (len <= 0) {
            break;
        }
    } while (IsET);
    return len;
}

ssize_t HttpConn::Write(int* err) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iov_cnt_);
        if (len <= 0) {
            *err = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            // write结束
            break;
        } else if (len > iov_[0].iov_len) {
            // 传输了len长度的数据，len > iov[0].len
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                write_buf_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = (uint8_t*) iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            write_buf_.Retrieve(len);
        }
    } while (IsET || ToWriteBytes() > 10240);
    return len;
}

bool HttpConn::Process() {
    // 解析请求并初始化响应
    request_.Init(); // 如果不进行Init，则会有之前Http request的数据残余
    if (read_buf_.ReadableBytes() <= 0) {
        return false;
    } else if (request_.Parse(read_buf_)) {
        LOG_DEBUG("%s", request_.Path().c_str());
        response_.Init(SrcDir, request_.Path(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(SrcDir, request_.Path(), false, 400);
    }

    // 处理响应，http响应保存到了write_buf中
    response_.MakeResponse(write_buf_);
    iov_[0].iov_base = const_cast<char*>(write_buf_.Peek());
    iov_[0].iov_len = write_buf_.ReadableBytes();
    iov_cnt_ = 1;
    if (response_.FileLen() > 0 && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iov_cnt_ = 2;
    }
    LOG_DEBUG("File size: %d", response_.FileLen());
    return true;
}

int HttpConn::GetFd() const {
    return fd_;
}

int HttpConn::GetPort() const {
    return addr_.sin_port;
}

const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

struct sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

bool HttpConn::IsKeepAlive() const {
    return request_.IsKeepAlive();
}

int HttpConn::ToWriteBytes() {
    return iov_[0].iov_len + iov_[1].iov_len;
}
