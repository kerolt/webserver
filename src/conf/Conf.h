#pragma once

#include <string>
#include <fstream>
#include <map>
#include <iostream>

class Conf {
public:
    Conf();
    void init(char* path);
    int GetIoThreads();
    int GetListenq();
    int GetLogLevel();
    int GetCapacity();
    int GetKeepAlived();
    bool GetSSL();
    std::string GetStorage();
    std::string GetPort();
    std::string GetLogFile();
    std::string GetSSLCrtPath();
    std::string GetSSLKeyPath();

private:
    int GetPos(std::string& buf, int start, int end, bool flag);
    void SeparateKV(std::map<std::string, std::string>& m, std::string& buf);
    void SolveComment(std::string& buf);

private:
    char conf[100];
    int io_thread;
    int listenq;
    int keep_alived;
    int capacity;
    int log_level_;
    bool ssl;
    std::string port;
    std::string storage;
    std::string logfile;
    std::string sslcrtpath;
    std::string sslkeypath;
};

// int GetIoThreads();
// string GetPort();
// int GetListenq();
// int GetKeepAlived();
// string GetStorage();
// string GetLogFile();
Conf& GetConf();
