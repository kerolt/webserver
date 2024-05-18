#include "Conf.h"

#include <cstring>

#include <type_traits>

#define DEFAULT_IO_THREAD 3
#define DEFAULT_PORT "8080"
#define DEFAULT_LISTENQ 2048
#define DEFAULT_STORAGE "page/"
#define DEFAULT_KEEP_ALIVED 20000
#define DEFAULT_LOGFILE "log/server.log"
#define DEFAULT_LOG_LEVEL 1
#define DEFAULT_CAPACITY 10
#define DEFAULT_SSL false
#define DEFAULT_SSLCRTPATH "ssl/ca.crt"
#define DEFAULT_SSLKEYPATH "ssl/ca.key"

int Conf::GetIoThreads() {
    return io_thread_;
}

std::string Conf::GetPort() {
    return port_;
}

int Conf::GetListenq() {
    return listenq_;
}

int Conf::GetKeepAlived() {
    return keep_alived_;
}

std::string Conf::GetStorage() {
    return storage_;
}

std::string Conf::GetLogFile() {
    return logfile_;
}

int Conf::GetCapacity() {
    return capacity_;
}

bool Conf::GetSSL() {
    return ssl_;
}

std::string Conf::GetSSLCrtPath() {
    return sslcrtpath_;
}

std::string Conf::GetSSLKeyPath() {
    return sslkeypath_;
}

int Conf::GetLogLevel() {
    return log_level_;
}

int Conf::GetPos(std::string& buf, int start, int end, bool flag) {
    for (; start < end; ++start) {
        if (flag) {
            if (' ' == buf[start] || '\t' == buf[start])
                return start;
        } else {
            if (' ' != buf[start] && '\t' != buf[start])
                return start;
        }
    }
    return start;
}

void Conf::SeparateKV(std::map<std::string, std::string>& m, std::string& buf) {
    int kstart = GetPos(buf, 0, buf.size(), false);
    int kend = GetPos(buf, kstart, buf.size(), true);
    int vstart = GetPos(buf, kend, buf.size(), false);
    int vend = GetPos(buf, vstart, buf.size(), true);
    if (vstart != buf.size())
        m[buf.substr(kstart, kend - kstart)] = buf.substr(vstart, vend - vstart);
}

void Conf::SolveComment(std::string& buf) {
    int last = buf.find("#", 0);
    if (std::string::npos == last)
        return;
    buf = buf.substr(0, last);
    return;
}

Conf::Conf() {
}

// 类型萃取简化if else代码
template<typename T>
T GetValueOrDefault(const std::map<std::string, std::string>& m, const std::string& key, const T& default_value) {
    auto it = m.find(key);
    if (it != m.end()) {
        if constexpr (std::is_integral_v<T>) {
            return std::stoi(it->second);
        } else if constexpr (std::is_same_v<T, bool>) {
            return it->second == "1";
        } else if constexpr (std::is_same_v<T, std::string>) {
            return it->second;
        } else {
            return it->second;
        }
    }
    return default_value;
}

void Conf::Init(char* path) {
    strncpy(conf_, path, 99);
    std::map<std::string, std::string> m;
    std::fstream file;
    file.open(conf_, std::ios::in);
    std::string buf;
    while (!file.eof()) {
        getline(file, buf);
        SolveComment(buf);
        if ("" == buf)
            continue;
        SeparateKV(m, buf);
    }
    file.close();

    io_thread_ = GetValueOrDefault(m, "io_thread", DEFAULT_IO_THREAD);
    port_ = GetValueOrDefault(m, "port", std::string(DEFAULT_PORT));
    listenq_ = GetValueOrDefault(m, "listenq", DEFAULT_LISTENQ);
    storage_ = GetValueOrDefault(m, "storage", std::string(DEFAULT_STORAGE));
    keep_alived_ = GetValueOrDefault(m, "keep-alived", DEFAULT_KEEP_ALIVED);
    logfile_ = GetValueOrDefault(m, "logfile", std::string(DEFAULT_LOGFILE));
    capacity_ = GetValueOrDefault(m, "capacity", DEFAULT_CAPACITY);
    ssl_ = GetValueOrDefault(m, "ssl", DEFAULT_SSL);
    sslcrtpath_ = GetValueOrDefault(m, "sslcrtpath", std::string(DEFAULT_SSLCRTPATH));
    sslkeypath_ = GetValueOrDefault(m, "sslkeypath", std::string(DEFAULT_SSLKEYPATH));
    log_level_ = GetValueOrDefault(m, "log_level", DEFAULT_LOG_LEVEL);
}

Conf& GetConf() {
    static Conf conf;
    return conf;
}
