#pragma once

#include <vector>

#include "LogStream.h"
#include "LogFile.h"
#include "FixedBuffer.h"
#include "../thread_pool/Thread.h"
#include "../mutex/MutexLock.h"
#include "../mutex/Condition.h"

class AsyncLogging : noncopyable {
public:
    AsyncLogging(const std::string logFileName, int FlushInterval = 2);
    ~AsyncLogging();
    void Append(const char* logfile, int len);
    void start();
    void stop();

private:
    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferVector = std::vector<std::shared_ptr<Buffer>>;
    using BufferPtr = std::shared_ptr<Buffer>;

    void ThreadFunc();

private:
    const int flush_interval_;
    bool is_running_;
    std::string basename_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    BufferPtr current_buffer_;
    BufferPtr next_buffer_;
    BufferVector buffers_;
};

typedef std::unique_ptr<AsyncLogging, decltype(DeleteElement<AsyncLogging>)*> UP_AsyncLogging;
