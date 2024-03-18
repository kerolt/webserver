#include "log.h"

#include <assert.h>
#include <iostream>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <memory>
#include <mutex>
#include <thread>

#include "block_queue.h"

Log::Log()
    : is_async_(false)
    , write_thread_(nullptr)
    , queue_(nullptr)
    , file_(nullptr) {}

Log::~Log() {
    // 等待写线程完成任务
    if (write_thread_ != nullptr && write_thread_->joinable()) {
        while (!queue_->Empty()) {
            queue_->Flush();
        }
        queue_->Close();
        write_thread_->join();
    }
    if (file_) {
        std::lock_guard<std::mutex> lock(mutex_);
        // Flush();
        fflush(file_);
        fclose(file_);
    }
}

Log* Log::Instance() {
    static Log instance;
    return &instance;
}

void Log::Init(const char* path, const char* suffix, int max_queue_size, int level) {
    is_open_ = true;
    level_ = level; // 默认 INFO 级别
    if (max_queue_size > 0) {
        is_async_ = true;
        if (!queue_) {
            write_thread_ = std::make_unique<std::thread>(AsyncThread);
            queue_ = std::make_unique<BlockQueue<std::string>>(max_queue_size);
        }
    } else {
        is_async_ = false;
    }

    lines_ = 0;
    path_ = path;
    suffix_ = suffix;

    time_t timer = time(nullptr);
    tm* sys_time = localtime(&timer);
    char file_name[kLogNameLen]{};
    snprintf(file_name, kLogNameLen - 1, "%s/%04d_%02d_%02d%s",
             path_,
             sys_time->tm_year + 1900,
             sys_time->tm_mon + 1,
             sys_time->tm_mday,
             suffix_);
    day_ = sys_time->tm_mday;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_.RetrieveAll();
        if (file_) {
            Flush();
            fclose(file_);
        }
        file_ = fopen(file_name, "a");
        if (file_ == nullptr) {
            mkdir(file_name, 0777);
            file_ = fopen(file_name, "a");
        }
        assert(file_ != nullptr);
    }
}

// 向缓冲区中写入数据，真正将数据写入文件将由写线程来完成
void Log::Write(int level, const char* format, ...) {
    timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t sec = now.tv_sec;
    tm* sys_time = localtime(&sec);
    va_list args;

    if (day_ != sys_time->tm_mday || (lines_ && (lines_ % kMaxLines == 0))) {
        std::lock_guard<std::mutex> lock(mutex_);

        // 记录日志的时间不一致，使用新的文件名
        char new_log[kLogNameLen];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", sys_time->tm_year + 1900, sys_time->tm_mon + 1, sys_time->tm_mday);
        if (day_ != sys_time->tm_mday) {
            // 文件名如：path/2024_02_15.log
            snprintf(new_log, 255, "%s/%s%s", path_, tail, suffix_);
            day_ = sys_time->tm_mday;
            lines_ = 0;
        } else {
            snprintf(new_log, 255, "%s/%s-%d%s", path_, tail, (lines_ / kMaxLines), suffix_);
        }

        Flush();
        fclose(file_);
        file_ = fopen(new_log, "a");
        assert(file_ != nullptr);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        lines_++;
        int n = snprintf(buffer_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         sys_time->tm_year + 1900, sys_time->tm_mon + 1, sys_time->tm_mday,
                         sys_time->tm_hour, sys_time->tm_min, sys_time->tm_sec, now.tv_usec);
        buffer_.HasWritten(n);
        AppendLogLevelTitle(level);

        va_start(args, format);
        int m = vsnprintf(buffer_.BeginWrite(), buffer_.WritableBytes(), format, args);
        va_end(args);

        buffer_.HasWritten(m);
        buffer_.Append("\n\0", 2); // fputs读取数据直到遇见'\0'

        if (is_async_ && queue_ && !queue_->Full()) {
            queue_->Push(buffer_.RetrieveAllToStr());
        } else {
            fputs(buffer_.Peek(), file_);
        }
        buffer_.RetrieveAll();
    }
}

void Log::Flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_async_) {
        queue_->Flush();
    }
    fflush(file_);
}

void Log::AsyncThread() {
    Log::Instance()->AsyncWrite();
}

void Log::AsyncWrite() {
    std::string str{};
    while (queue_->Pop(str)) {
        std::lock_guard<std::mutex> lock(mutex_);
        fputs(str.c_str(), file_);
    }
}

void Log::AppendLogLevelTitle(int level) {
    switch (level) {
        case 0:
            buffer_.Append("DEBUG : ", 8);
            break;
        case 1:
            buffer_.Append(" INFO : ", 8);
            break;
        case 2:
            buffer_.Append(" WARN : ", 8);
            break;
        case 3:
            buffer_.Append("ERROR : ", 8);
            break;
        default:
            buffer_.Append(" INFO : ", 8);
            break;
    }
}

void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

int Log::GetLevel() {
    std::lock_guard<std::mutex> lock(mutex_);
    return level_;
}

bool Log::IsOpen() {
    return is_open_;
}
