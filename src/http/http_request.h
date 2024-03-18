#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "buffer.h"

class HttpRequest {
public:
    enum class ParseState {
        REQUEST_LINE, // 请求行
        HEADERS,      // 请求头
        BODY,         // 请求体
        FINISH        // 完成状态
    };

    enum class HttpCode {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        NO_RESOURSE,
        CLOSE_CONNECTION,
        INTERNAL_ERROR
    };

public:
    HttpRequest() {
        method_ = path_ = version_ = body_ = "";
        state_ = ParseState::REQUEST_LINE;
        header_.clear();
        post_.clear();
    }
    ~HttpRequest() = default;

    bool Parse(Buffer& buffer);
    bool IsKeepAlive() const;

    std::string Path() const;
    std::string& Path();
    std::string Method() const;
    std::string Version() const;
    std::string GetPost(const std::string& key) const;
    std::string GetPost(const char* key) const;

private:
    static int ConvertHex(char ch);
    static bool UserVerify(const std::string& name, const std::string& pwd, bool is_login);

    bool ParseRequestLine(const std::string& line);
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);
    void ParsePath();
    void ParsePost();
    void ParseFromUrl();

private:
    ParseState state_;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> kDefaultHtml;
    static const std::unordered_map<std::string, int> kDefaultHtmlTag;
};

#endif /* HTTP_REQUEST_H_ */
