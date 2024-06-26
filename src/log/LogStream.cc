#include "LogStream.h"

template <typename T>
size_t convert(char buf[], T value) {
    char* p = buf;
    bool minus = value < 0 ? true : false;
    while (value) {
        *p++ = (value % 10) + '0';
        value /= 10;
    }
    if (minus)
        *p++ = '-';
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

template <typename T>
void LogStream::FormatInteger(T v) {
    if (buffer_.avail() >= kMaxNumbericSize) {
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

template <typename T>
void LogStream::FormatDecimal(T v) {
    if (buffer_.avail() >= kMaxNumbericSize) {
        int len = snprintf(buffer_.current(), kMaxNumbericSize, "%.12g", v);
        buffer_.add(len);
    }
}

LogStream& LogStream::operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
}

LogStream& LogStream::operator<<(short v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    FormatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    FormatDecimal(v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    FormatDecimal(v);
    return *this;
}

LogStream& LogStream::operator<<(long double v) {
    FormatDecimal(v);
    return *this;
}

LogStream& LogStream::operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
}

LogStream& LogStream::operator<<(const char* v) {
    if (v)
        buffer_.append(v, strlen(v));
    else
        buffer_.append("(null)", 6);
    return *this;
}

LogStream& LogStream::operator<<(const std::string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
}

void LogStream::append(const char* data, int len) {
    buffer_.append(data, len);
}

const LogStream::Buffer& LogStream::buffer() const {
    return buffer_;
}

void LogStream::resetBuffer() {
    buffer_.reset();
}
