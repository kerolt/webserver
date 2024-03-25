#include "http_request.h"

#include <cassert>
#include <algorithm>
#include <mysql/mysql.h>
#include <string>
#include <regex>
#include <unordered_set>

#include "log.h"
#include "sql_pool.h"

const std::unordered_set<std::string> HttpRequest::kDefaultHtml = {"/index", "/login", "/register", "/welcome"};

const std::unordered_map<std::string, int> HttpRequest::kDefaultHtmlTag = {{"/register.html", 0}, {"/login.html", 1}};

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

// TODO 可能将该功能移出Http Request做一个单独的模块
bool HttpRequest::UserVerify(const std::string& name, const std::string& pwd, bool is_login) {
}

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

// 路由与html的映射
void HttpRequest::ParsePath() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (auto& path : kDefaultHtml) {
            if (path == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

// 处理表单提交和用户验证
void HttpRequest::ParsePost() {
    if (method_ != "POST" || header_["Content-Type"] != "application/x-www-form-urlencoded") {
        return;
    }
    ParseFromUrl();
    if (kDefaultHtmlTag.count(path_)) {
        int tag = kDefaultHtmlTag.find(path_)->second;
        LOG_DEBUG("Tag: %d", tag);
        if (tag == 0 || tag == 1) {
            bool is_login = (tag == 1);
            if (UserVerify(post_["username"], post_["password"], is_login)) {
                path_ = "/welcome.html";
            } else {
                path_ = "/error.html";
            }
        }
    }
}

// 解析在URL中的参数
// 键值对使用 & 隔开，+ 代表空格
void HttpRequest::ParseFromUrl() {
    if (body_.size() == 0) {
        return;
    }

    std::string key, value;
    int num{};
    int body_size = body_.size();
    int left{}, right{};

    while (right < body_size) {
        char ch = body_[right];
        switch (ch) {
            case '=': // 键 结束的位置
                key = body_.substr(left, right - left);
                left = right + 1;
                break;
            case '+':
                body_[right] = ' ';
                break;
            case '%':
                num = ConvertHex(body_[right + 1]) * 16 + ConvertHex(body_[right + 2]);
                body_[right + 2] = num % 10 + '0';
                body_[right + 1] = num / 10 + '0';
                break;
            case '&': // 值 结束的位置
                value = body_.substr(left, right - left);
                post_[key] = value;
                LOG_DEBUG("key-value: %s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
        right++;
    }
    assert(left <= right);
    if (post_.count(key) == 0 && left < right) {
        value = body_.substr(left, right - left);
        post_[key] = value;
    }
}