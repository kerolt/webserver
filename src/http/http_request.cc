#include "http_request.h"

#include <cassert>
#include <algorithm>
#include <string>
#include <regex>

#include "log.h"

// 解析HTTP请求
bool HttpRequest::Parse(Buffer& buffer) {
    const char crlf[] = "\r\n";
    if (buffer.ReadableBytes() <= 0) {
        return false;
    }
    while (buffer.ReadableBytes() && state_ != ParseState::FINISH) {
        const char* line_end = std::search(buffer.Peek(), buffer.BeginWriteConst(), crlf, crlf + 1);
        std::string line(buffer.Peek(), line_end);
        switch (state_) {
            case ParseState::REQUEST_LINE:
                if (!ParseRequestLine(line)) {
                    return false;
                }
                ParsePath();
                break;
            case ParseState::HEADERS:
                ParseHeader(line);
                if (buffer.ReadableBytes() <= 2) {
                    state_ = ParseState::FINISH;
                }
                break;
            case ParseState::BODY:
                ParseBody(line);
                break;
            default:
                break;
        }
        if (line_end == buffer.BeginWrite()) {
            break;
        }
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

// 检查Http是否保持连接状态
bool HttpRequest::IsKeepAlive() const {
    auto iterator = header_.find("Connection");
    if (iterator != header_.end()) {
        return iterator->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

std::string HttpRequest::Path() const {
    return path_;
}

std::string& HttpRequest::Path() {
    return path_;
}

std::string HttpRequest::Method() const {
    return method_;
}

std::string HttpRequest::Version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    auto iterator = post_.find(key);
    return iterator != post_.end() ? iterator->second : "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    auto iterator = post_.find(key);
    return iterator != post_.end() ? iterator->second : "";
}

int HttpRequest::ConvertHex(char ch) {
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return ch;
}

bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool is_login) {}

// 解析HTTP请求的第一行
// METHOD PATH HTTP/VERSION
bool HttpRequest::ParseRequestLine(const std::string& line) {
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_smatch;
    if (std::regex_match(line, sub_smatch, patten)) {
        method_ = sub_smatch[0];
        path_ = sub_smatch[1];
        version_ = sub_smatch[2];
        state_ = ParseState::HEADERS;
        return true;
    }
    LOG_ERROR("Request line error!");
    return false;
}

// 解析请求头
void HttpRequest::ParseHeader(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        header_[sub_match[1]] = sub_match[2];
    } else {
        state_ = ParseState::BODY;
    }
}

// 保存POST的请求体
void HttpRequest::ParseBody(const std::string& line) {
    body_ = line;
    ParsePost();
    state_ = ParseState::FINISH;
    // 日志级别默认INFO
    // 实际生产中不需要记录body的日志，所以级别设置DEBUG即可
    LOG_DEBUG("\"Body\": %s; \"Len\": %d", line.c_str(), line.size());
}

void HttpRequest::ParsePath() {}

void HttpRequest::ParsePost() {}

void HttpRequest::ParseFromUrl() {}