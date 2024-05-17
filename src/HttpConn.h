#pragma once

#include <sys/mman.h>

#include <string>
#include <iostream>
#include <unordered_map>

#include "Packet.h"
#include "Mimetype.h"
#include "reactor/EventLoop.h"
#include "conf/Conf.h"
#include "memory_pool/MemoryPool.h"
#include "cache/LFUCache.h"

enum Method {
    METHOD_GET,
    METHOD_POST
};

enum HttpVersion {
    HTTP_10,
    HTTP_11
};

enum ParseState {
    PARSE_ERROR,
    PARSE_METHOD,
    PARSE_HEADER,
    PARSE_SUCCESS
};

class Channel;

class HttpConn {
public:
    explicit HttpConn(std::shared_ptr<Channel> channel);
    ~HttpConn() = default;

private:
    using CallBack = std::function<ParseState()>;

    ParseState ParseMethod();
    ParseState ParseHeader();
    ParseState ParseError();
    ParseState ParseSuccess();

    void Parse();
    void Send();
    bool Read(string& msg, const string& str);
    void InitMsg();
    void HandleError(int errornum, const string& msg);

private:
    bool is_keepalive_;
    int pos_;
    int size_;
    std::shared_ptr<Channel> channel_;
    std::string in_buffer_;
    std::string storage_;
    std::string path_;
    std::string filetype_;
    CallBack handle_parse_[4];
    Method method_;
    HttpVersion version_;
    ParseState parse_state_;
    std::unordered_map<string, string> header_;
};
