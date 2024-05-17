#pragma once

#include "FileUtil.h"
#include "../mutex/MutexLock.h"
#include "../memory_pool/MemoryPool.h"

class LogFile : noncopyable {
public:
    explicit LogFile(std::string Basename, int FlushEveryN = 1024);
    ~LogFile() = default;
    void Append(const char* logline, int len);
    void Flush();
    bool rollFile();

private:
    void AppendUnlocked(const char* logline, int len);
    const std::string basename;
    const int flushEveryN;
    int count;
    MutexLock mutex;
    std::unique_ptr<FileUtil, decltype(DeleteElement<FileUtil>)*> file;
};
