#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <cstddef>
#include <sys/stat.h>
#include <string>
#include <unordered_map>
#include "buffer.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& src, std::string& path, bool is_alive = false, int code = -1);
    void MakeResponse(Buffer& buf);
    void UnmapFile();
    void ErrorContent(Buffer& buf, std::string msg);

    char* File() {
        return file_;
    }

    size_t FileLen() const {
        return file_state_.st_size;
    }

    int Code() const {
        return code_;
    }

private:
    void AddStateLine(Buffer& buf);
    void AddHeader(Buffer& buf);
    void AddContent(Buffer& buf);

    void ErrorHtml();
    std::string GetFileType();

private:
    static const std::unordered_map<std::string, std::string> kSuffixType;
    static const std::unordered_map<int, std::string> kCodeStatus;
    static const std::unordered_map<int, std::string> kCodePath;

    int code_;
    char* file_;
    bool is_alive_;
    std::string path_;
    std::string src_dir_;
    struct stat file_state_;
};

#endif /* HTTP_RESPONSE_H_ */
