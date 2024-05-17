#include "AsyncLogging.h"

AsyncLogging::AsyncLogging(const std::string logFileName, int FlushInterval)
    : flush_interval_(FlushInterval)
    , is_running_(false)
    , basename_(logFileName)
    , thread_(std::bind(&AsyncLogging::ThreadFunc, this), "Logging")
    , mutex_()
    , cond_(mutex_)
    , current_buffer_(NewElement<Buffer>(), DeleteElement<Buffer>)
    , next_buffer_(NewElement<Buffer>(), DeleteElement<Buffer>) {
}

AsyncLogging::~AsyncLogging() {
    if (is_running_) {
        stop();
    }
}

void AsyncLogging::Append(const char* logfile, int len) {
    MutexLockGuard lock(mutex_);
    if (current_buffer_->avail() > len) {
        current_buffer_->append(logfile, len);
    } else {
        buffers_.emplace_back(current_buffer_);
        if (next_buffer_) {
            current_buffer_ = std::move(next_buffer_);

        } else {
            current_buffer_.reset(NewElement<Buffer>(), DeleteElement<Buffer>);
        }
        current_buffer_->append(logfile, len);
        cond_.notify();
    }
}

void AsyncLogging::ThreadFunc() {
    BufferPtr new_buffer1(NewElement<Buffer>(), DeleteElement<Buffer>);
    BufferPtr new_buffer2(NewElement<Buffer>(), DeleteElement<Buffer>);
    BufferVector buffers_to_write;
    LogFile output(basename_);
    while (is_running_) {
        {
            MutexLockGuard lock(mutex_);
            if (buffers_.empty()) {
                cond_.waitForSeconds(flush_interval_);
            }
            buffers_.emplace_back(current_buffer_);
            current_buffer_ = std::move(new_buffer1);
            buffers_to_write.swap(buffers_);
            if (!next_buffer_) {
                next_buffer_ = std::move(new_buffer2);
            }
        }
        for (auto& wi : buffers_to_write) {
            output.Append(wi->data(), wi->length());
        }
        if (!new_buffer1) {
            new_buffer1.reset(NewElement<Buffer>(), DeleteElement<Buffer>);
        }
        if (!new_buffer2) {
            new_buffer2.reset(NewElement<Buffer>(), DeleteElement<Buffer>);
        }
        buffers_to_write.clear();
        output.Flush();
    }
    output.Flush();
}

void AsyncLogging::start() {
    is_running_ = true;
    thread_.start();
}

void AsyncLogging::stop() {
    is_running_ = false;
    cond_.notify();
    thread_.join();
}
