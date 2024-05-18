#pragma once

#include <cstring>

#include "../noncopyable.h"

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1024;

template <int SIZE>
class FixedBuffer : noncopyable {
public:
    FixedBuffer()
        : cur(data_) {}
    ~FixedBuffer() = default;

    void append(const char* buf, int len) {
        if (avail() > len) {
            memcpy(cur, buf, len);
            cur += len;
        }
    }
    const char* data() const {
        return data_;
    }
    int length() const {
        return cur - data_;
    }
    char* current() {
        return cur;
    }
    int avail() const {
        return data_ + SIZE - cur;
    }
    void add(int len) {
        cur += len;
    }
    void reset() {
        cur = data_;
    }
    void bzero() {
        memset(data_, 0, SIZE);
    }

private:
    const char* end() const {
        return data_ + sizeof(data_);
    }

    char data_[SIZE]{};
    char* cur;
};
