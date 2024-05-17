#pragma once

#include <sys/time.h>

#include "AsyncLogging.h"
#include "FixedBuffer.h"
#include "../conf/Conf.h"

class Impl {
public:
    Impl(const char* filename, int line, int level);
    void formatTime();
    int GetLine() const;
    LogStream& stream();
    std::string GetBaseName();

private:
    int line;
    int level_;
    LogStream stream_;
    std::string basename_;
};

class Logger {
public:
    Logger(const char* filename, int line, int level);
    ~Logger();
    LogStream& Stream();
    // static void setLogFileName(std::string fileName);
    static std::string GetLogFileName();

    static std::string logFileName;

private:
    Impl impl_;
    int level_;
};

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

#define LOG(level) Logger(__FILE__, __LINE__, level).Stream()
#define LOG_DEBUG LOG(0) << "DEBUG --- "
#define LOG_INFO LOG(1) << " INFO --- "
#define LOG_WARN LOG(2) << " WARN --- "
#define LOG_ERROR LOG(3) << "ERROR --- "