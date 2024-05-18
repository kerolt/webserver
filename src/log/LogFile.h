#pragma once

#include <atomic>

#include "FileUtil.h"
#include "../mutex/MutexLock.h"
#include "../memory_pool/MemoryPool.h"

const int kRollSize = 5000;

class LogFile : noncopyable {
public:
    LogFile(std::string basename, int roll_size = kRollSize, int flush_interval = 3, int flush_every_n = 1024);
    ~LogFile() = default;
    void Append(const char* logline, int len);
    void Flush();
    bool RollFile();

    static std::atomic_int count;

private:
    static std::string GetLogFileName(const std::string basename, time_t* now);

    void AppendUnlocked(const char* logline, int len);

private:
    static const int kRollSeconds_ = 60 * 60 * 24;

    std::string basename_;
    const off_t roll_size_;
    const int flush_every_n_;
    const int flush_interval_;


    std::unique_ptr<FileUtil, decltype(DeleteElement<FileUtil>)*> file_;
    MutexLock mutex_;

    time_t start_period_;
    time_t last_roll_;
    time_t last_flush_;
};
