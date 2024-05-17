#include "Logging.h"

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

std::string Logger::logFileName;

UP_AsyncLogging& GetAsyncLogging() {
    static UP_AsyncLogging AsyncLogger(NewElement<AsyncLogging>(Logger::GetLogFileName()), DeleteElement<AsyncLogging>);
    return AsyncLogger;
}

void Init() {
    Logger::logFileName = GetConf().GetLogFile();
    GetAsyncLogging()->start();
}

void Output(const char* msg, int len) {
    pthread_once(&once_control, Init);
    // if (Logger::level)
    GetAsyncLogging()->Append(msg, len);
}

Impl::Impl(const char* fileName, int line, int level)
    : line(line)
    , basename_(fileName)
    , level_(level) {
    formatTime();
}

void Impl::formatTime() {
    if (level_ < GetConf().GetLogLevel()) {
        return;
    }
    struct timeval tv {};
    time_t time;
    char str_t[26] = {0};
    gettimeofday(&tv, nullptr);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S ", p_time);
    stream_ << str_t;
}

int Impl::GetLine() const {
    return line;
}

LogStream& Impl::stream() {
    return stream_;
}

std::string Impl::GetBaseName() {
    return basename_;
}

Logger::Logger(const char* fileName, int line, int level)
    : level_(level)
    , impl_(fileName, line, level) {
}

Logger::~Logger() {
    if (level_ < GetConf().GetLogLevel()) {
        return;
    }
    Stream() << "\t[" << impl_.GetBaseName() << ':' << impl_.GetLine() << "]\n";
    const LogStream::Buffer& buf(impl_.stream().buffer());
    Output(buf.data(), buf.length());
}

/*void Logger::setLogFileName(std::string fileName){
    logFileName=fileName;
}*/

std::string Logger::GetLogFileName() {
    return logFileName;
}

LogStream& Logger::Stream() {
    return impl_.stream();
}
