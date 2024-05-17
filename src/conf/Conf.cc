#include <cstring>
#include "Conf.h"

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
    return io_thread;
}

std::string Conf::GetPort() {
    return port;
}

int Conf::GetListenq() {
    return listenq;
}

int Conf::GetKeepAlived() {
    return keep_alived;
}

std::string Conf::GetStorage() {
    return storage;
}

std::string Conf::GetLogFile() {
    return logfile;
}

int Conf::GetCapacity() {
    return capacity;
}

bool Conf::GetSSL() {
    return ssl;
}

std::string Conf::GetSSLCrtPath() {
    return sslcrtpath;
}

std::string Conf::GetSSLKeyPath() {
    return sslkeypath;
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

void Conf::init(char* path) {
    strncpy(conf, path, 99);
    std::map<std::string, std::string> m;
    std::fstream file;
    file.open(conf, std::ios::in);
    std::string buf;
    while (!file.eof()) {
        getline(file, buf);
        SolveComment(buf);
        if ("" == buf)
            continue;
        SeparateKV(m, buf);
    }
    file.close();

    if (m.find("IO_thread") != m.end())
        io_thread = stoi(m["IO_thread"]);
    else
        io_thread = DEFAULT_IO_THREAD;

    if (m.find("port") != m.end())
        port = m["port"];
    else
        port = DEFAULT_PORT;

    if (m.find("listenq") != m.end())
        listenq = stoi(m["listenq"]);
    else
        listenq = DEFAULT_LISTENQ;

    if (m.find("storage") != m.end())
        storage = m["storage"];
    else
        storage = DEFAULT_STORAGE;

    if (m.find("keep-alived") != m.end())
        keep_alived = stoi(m["keep-alived"]);
    else
        keep_alived = DEFAULT_KEEP_ALIVED;

    if (m.find("logfile") != m.end())
        logfile = m["logfile"];
    else
        logfile = DEFAULT_LOGFILE;

    if (m.find("capacity") != m.end())
        capacity = stoi(m["capacity"]);
    else
        capacity = DEFAULT_CAPACITY;

    if (m.find("ssl") != m.end())
        ssl = ("1" == m["ssl"]);
    else
        ssl = DEFAULT_SSL;

    if (m.find("sslcrtpath") != m.end())
        sslcrtpath = m["sslcrtpath"];
    else
        sslcrtpath = DEFAULT_SSLCRTPATH;

    if (m.find("sslkeypath") != m.end())
        sslkeypath = m["sslkeypath"];
    else
        sslkeypath = DEFAULT_SSLKEYPATH;

    if (m.find("log_level") != m.end()) {
        log_level_ = stoi(m["log_level"]);
    } else {
        log_level_ = DEFAULT_LOG_LEVEL;
    }
}

Conf& GetConf() {
    static Conf conf;
    return conf;
}
