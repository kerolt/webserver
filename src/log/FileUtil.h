#pragma once

#include <fstream>
#include <memory>

#include "../memory_pool/MemoryPool.h"
#include "../noncopyable.h"

constexpr int kBufferSize = 64 * 1024;

class FileUtil : noncopyable {
public:
    explicit FileUtil(const std::string& filename);
    ~FileUtil();
    void Append(const char* log_line, int len);
    void Flush();

private:
    int Write(const char* log_line, int len);

private:
    FILE* fp_;
    char buffer[kBufferSize]{};
};
