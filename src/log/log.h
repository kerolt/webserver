#ifndef LOG_H_
#define LOG_H_

#include <memory>
#include <thread>
#include <mutex>

#include "block_queue.h"
#include "buffer.h"

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Log {
public:
    static Log* Instance();
    static void AsyncThread();

    void Init(
        const char* path = "./log",
        const char* suffix = ".log",
        int max_queue_size = 1024,
        int level = INFO);
    void Write(int level, const char* filename, const char* funcname, int line, const char* format, ...);
    void Flush();
    void SetLevel(int level);
    int GetLevel();
    bool IsOpen();

private:
    Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    Log(Log&&) = delete;
    virtual ~Log();

    void AppendLogLevelTitle(int level);
    void AsyncWrite();

private:
    static const int kLogPathLen = 256;
    static const int kLogNameLen = 256;
    static const int kMaxLines = 50000;

    const char* path_;
    const char* suffix_;

    int lines_;
    int day_;

    int level_;
    bool is_open_;
    bool is_async_;
    Buffer buffer_;

    FILE* file_;

    std::mutex mutex_;
    std::unique_ptr<BlockQueue<std::string>> queue_;
    std::unique_ptr<std::thread> write_thread_;
};

// 日志等级分为0,1,2,3
// 能够记录的日志等级必须在初始化时给出的等级及以上
#define LOG_BASE(level, format, ...)                                                                            \
    do {                                                                                                        \
        Log* log = Log::Instance();                                                                             \
        if (log->IsOpen() && log->GetLevel() <= level) {                                                        \
            log->Write(level, (char*) (__FILE__), (char*) (__func__), (int) (__LINE__), format, ##__VA_ARGS__); \
            log->Flush();                                                                                       \
        }                                                                                                       \
    } while (0);

#define LOG_DEBUG(format, ...) LOG_BASE(DEBUG, format, ##__VA_ARGS__);
#define LOG_INFO(format, ...) LOG_BASE(INFO, format, ##__VA_ARGS__);
#define LOG_WARN(format, ...) LOG_BASE(WARN, format, ##__VA_ARGS__);
#define LOG_ERROR(format, ...) LOG_BASE(ERROR, format, ##__VA_ARGS__);

#endif /* LOG_H_ */
