#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include "buffer.h"
#include <sys/stat.h>
#include <string>
#include <unordered_map>

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

private:
    void AddHeader(Buffer& buffer);
    void AddContent(Buffer& buffer);

private:
    static const std::unordered_map<std::string, std::string> kSuffixType;
    static const std::unordered_map<int, std::string> kCodeStatus;
    static const std::unordered_map<int, std::string> kCodePath;

    int code_;
    char* file_;
    bool is_keepalive;
    std::string path_;
    std::string src_dir_;
    struct stat file_state_;
};

#endif /* HTTP_RESPONSE_H_ */
