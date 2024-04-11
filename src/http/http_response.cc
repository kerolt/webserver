#include "http_response.h"
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string>
#include "log.h"

const std::unordered_map<std::string, std::string> HttpResponse::kSuffixType = {
    {"default", "text/plain"},
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/msword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::kCodeStatus = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const std::unordered_map<int, std::string> HttpResponse::kCodePath = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse() {
    code_ = -1;
    path_ = src_dir_ = "";
    is_alive_ = false;
    file_ = nullptr;
    file_state_ = {0};
}

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const std::string& src, std::string& path, bool is_alive, int code) {
    assert(src_dir_ != "");
    if (file_) {
        UnmapFile();
    }
    code_ = code;
    is_alive_ = is_alive;
    path_ = path;
    src_dir_ = src;
    file_ = nullptr;
    file_state_ = {0};
}

void HttpResponse::MakeResponse(Buffer& buf) {
    if (stat((src_dir_ + path_).data(), &file_state_) < 0 || S_ISDIR(file_state_.st_mode)) {
        code_ = 404; // 资源不存在
    } else if (!(file_state_.st_mode & S_IROTH)) {
        code_ = 403; // 资源存在但是访问不允许（不具备读权限）
    } else if (code_ == -1) {
        code_ = 200;
    }
    ErrorHtml();
    AddStateLine(buf);
    AddHeader(buf);
    AddContent(buf);
}

void HttpResponse::UnmapFile() {
    if (file_) {
        munmap(file_, file_state_.st_size);
        file_ = nullptr;
    }
}

void HttpResponse::ErrorContent(Buffer& buf, std::string msg) {
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (kCodeStatus.count(code_) == 1) {
        status = kCodeStatus.at(code_);
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + msg + "</p>";
    body += "<hr><em>WebServer</em></body></html>";

    buf.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buf.Append(body);
}

void HttpResponse::AddStateLine(Buffer& buf) {
    std::string status;
    if (kCodeStatus.count(code_)) {
        status = kCodeStatus.at(code_);
    } else {
        code_ = 400;
        status = kCodeStatus.at(400);
    }
    buf.Append("HTTP/1.1" + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader(Buffer& buf) {
    buf.Append("Connection: ");
    if (is_alive_) {
        buf.Append("keep-alive\r\n");
        buf.Append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buf.Append("close\r\n");
    }
    buf.Append("Content-type: " + GetFileType() + "\r\n");
}

void HttpResponse::AddContent(Buffer& buf) {
    int fd = open((src_dir_ + path_).data(), O_RDONLY);
    if (fd < 0) {
        LOG_ERROR("Failed to stat file: %s, error: %s", (src_dir_ + path_).data(), strerror(errno));
        ErrorContent(buf, "File Not Found!");
        close(fd);
        return;
    }

    LOG_DEBUG("file path: %s", (src_dir_ + path_).data());
    int* ret = (int*) mmap(0, file_state_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (*ret == -1) {
        LOG_ERROR("Failed to stat file: %s, error: %s", (src_dir_ + path_).data(), strerror(errno));
        ErrorContent(buf, "File Not Found!");
        close(fd);
        return;
    }
    file_ = (char*) ret;
    close(fd);
    buf.Append("Content-length: " + std::to_string(file_state_.st_size) + "\r\n\r\n");
    // 文件内容的传输将在HttpConn中实现
}

void HttpResponse::ErrorHtml() {
    if (kCodePath.count(code_)) {
        path_ = kCodePath.at(code_);
        stat((src_dir_ + path_).data(), &file_state_);
    }
}

std::string HttpResponse::GetFileType() {
    auto idx = path_.find_last_of('.');
    if (idx == std::string::npos) {
        return kSuffixType.at("default");
    }
    std::string suffix = path_.substr(idx);
    if (kSuffixType.count(suffix)) {
        return kSuffixType.at(suffix);
    }
    return kSuffixType.at("default");
}