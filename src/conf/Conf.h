#pragma once

#include <string>
#include <fstream>
#include <map>
#include <iostream>

class Conf {
public:
    Conf();
    void Init(char* path);
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
    char conf_[100];
    int io_thread_;
    int listenq_;
    int keep_alived_;
    int capacity_;
    int log_level_;
    bool ssl_;
    std::string port_;
    std::string storage_;
    std::string logfile_;
    std::string sslcrtpath_;
    std::string sslkeypath_;
};

Conf& GetConf();
