#pragma once

#include <cstring>

#include <string>
#include <algorithm>

#include "FixedBuffer.h"

class LogStream : noncopyable {
public:
    using Buffer = FixedBuffer<kSmallBuffer>;

public:
    LogStream& operator<<(bool);
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);
    LogStream& operator<<(float);
    LogStream& operator<<(double);
    LogStream& operator<<(long double);
    LogStream& operator<<(char);
    LogStream& operator<<(const char*);
    LogStream& operator<<(const std::string&);

    void append(const char* data, int len);
    void resetBuffer();
    const Buffer& buffer() const;

private:
    void staticCheck();

    template <typename T>
    void FormatInteger(T);

    template <typename T>
    void FormatDecimal(T);

    Buffer buffer_;
    static const int kMaxNumbericSize = 32;
};
