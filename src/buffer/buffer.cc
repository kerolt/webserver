#include "buffer.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

Buffer::Buffer(size_t inital_size)
    : buffer_(inital_size)
    , read_pos_(0)
    , write_pos_(0) {}

size_t Buffer::ReadableBytes() const {
    return write_pos_ - read_pos_;
}

// 可写空间大小
size_t Buffer::WritableBytes() const {
    return buffer_.size() - write_pos_;
}

size_t Buffer::PrependableBytes() const {
    return read_pos_;
}

const char* Buffer::Peek() const {
    return Begin() + read_pos_;
}

char* Buffer::BeginWrite() {
    return Begin() + write_pos_;
}

const char* Buffer::BeginWriteConst() const {
    return Begin() + write_pos_;
}

void Buffer::Append(const char* str, size_t len) {
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const Buffer& buf) {
    // 将传入的buf中的可读区域的数据添加至本类的buffer中
    Append(buf.Peek(), buf.ReadableBytes());
}

void Buffer::HasWritten(size_t len) {
    write_pos_ += len;
}

void Buffer::EnsureWritable(size_t len) {
    // 若可写空间不足，则调用MakeSpace分配更大的空间
    if (WritableBytes() < len) {
        MakeSpace(len);
    }
    // 重新分配空间后判断一下可写空间是否大于len
    assert(WritableBytes() >= len);
}

ssize_t Buffer::WriteToFd(int fd, int* err_no) {
    // 既然是向fd中写入数据，那么这些数据在buffer中自然就是可读的状态
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if (len < 0) {
        *err_no = errno;
        return len;
    }
    read_pos_ += len;
    return len;
}

ssize_t Buffer::ReadFromFd(int fd, int* err_no) {
    char extra_buf[65536]{}; // 临时开辟64KB的缓冲区
    iovec vec[2];
    size_t writable_len = WritableBytes();
    vec[0].iov_base = Begin() + write_pos_;
    vec[0].iov_len = writable_len;
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof(extra_buf);

    // 将fd中的数据读入到了iovec中，其中vec[0]指向的是buffer的可写处，vec[1]指向的是extra_buf
    ssize_t len = readv(fd, vec, 2);
    if (len < 0) {
        // 错误
        *err_no = errno;
    } else if (len <= writable_len) {
        // buffer的可写空间足够，直接修改write_pos_
        write_pos_ += len;
    } else {
        // buffer空间不足，不过在之前已往buffer中写入了writable_len的数据，往extra_buf中写入了
        // len 大小的数据
        write_pos_ = buffer_.size();
        Append(extra_buf, len - writable_len);
    }
    return len;
}

void Buffer::Retrieve(size_t len) {
    if (len <= ReadableBytes()) {
        read_pos_ += len;
    } else {
        RetrieveAll();
    }
}

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string data(Peek(), ReadableBytes());
    RetrieveAll();
    return data;
}

// ------------private function-----------------

// 返回buffer的起始地址
const char* Buffer::Begin() const {
    return &*buffer_.begin();
}

char* Buffer::Begin() {
    return &*buffer_.begin();
}

void Buffer::MakeSpace(size_t len) {
    if (WritableBytes() + PrependableBytes() < len) {
        // 可写空间 + 预留空间 < len；重新分配buffer大小
        buffer_.resize(write_pos_ + len + 1);
    } else {
        // 空间足够，将 [read_pos, write_pos) 内的数据移动到 buffer begin 的位置
        size_t readable = ReadableBytes();
        std::copy(Begin() + read_pos_, Begin() + write_pos_, Begin());
        read_pos_ = 0;
        write_pos_ = read_pos_ + readable;
    }
}
