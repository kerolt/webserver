#include "LogFile.h"

#include <utility>
#include <iostream>

std::atomic_int LogFile::count{0};

LogFile::LogFile(std::string basename, int roll_size, int flush_interval, int flush_every_n)
    : basename_(std::move(basename))
    , roll_size_(roll_size)
    , flush_interval_(flush_interval)
    , flush_every_n_(flush_every_n)
    , last_roll_(0)
    , last_flush_(0)
    , start_period_(0)
    , file_(nullptr, nullptr) {
    time_t now = 0;
    std::string filename = GetLogFileName(basename_, &now);
    file_ = std::unique_ptr<FileUtil, decltype(DeleteElement<FileUtil>)*>(NewElement<FileUtil>(filename), DeleteElement<FileUtil>);
}

void LogFile::Append(const char* logline, int len) {
    MutexLockGuard lock(mutex_);
    AppendUnlocked(logline, len);
}

void LogFile::Flush() {
    file_->Flush();
}

bool LogFile::RollFile() {
    time_t now = 0;
    time_t start = now / kRollSeconds_ * kRollSeconds_;
    std::string filename = GetLogFileName(basename_, &now);

    if (now > last_roll_) {
        last_roll_ = now;
        last_flush_ = now;
        start_period_ = start;
        file_.reset(new FileUtil(filename));
        return true;
    }
    return false;
}

std::string LogFile::GetLogFileName(const std::string basename, time_t* now) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(nullptr);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof timebuf, "%Y_%m_%d-%H%M%S", &tm);
    filename += timebuf;
    filename += ".log";

    return filename;
}

void LogFile::AppendUnlocked(const char* logline, int len) {
    // ++count; // ++操作应该在 ~Logger() 中执行
    // TODO 给文件设置的滚动刷新行数为5000，but测试的时候文件行数有4万行和8万行的情况，不知道这是为什么，日后再看
    if (count >= roll_size_) {
        count = 0;
        file_->Flush();
        RollFile();        
    }
    file_->Append(logline, len);
    file_->Flush();
}
