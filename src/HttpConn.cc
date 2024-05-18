#include "HttpConn.h"
#include "log/Logging.h"

static unsigned char FromHex(unsigned char x) {
    unsigned char y;
    if (x >= 'A' && x <= 'Z') {
        y = x - 'A' + 10;
    } else if (x >= 'a' && x <= 'z') {
        y = x - 'a' + 10;
    } else if (x >= '0' && x <= '9') {
        y = x - '0';
    } else {
        assert(0);
    }
    return y;
}

static std::string UrlDecode(const std::string& str) {
    std::string str_temp;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '+') {
            // str_temp += ' ';
            continue;
        } else if (str[i] == '%') {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char) str[++i]);
            unsigned char low = FromHex((unsigned char) str[++i]);
            str_temp += high * 16 + low;
        } else {
            str_temp += str[i];
        }
    }
    return str_temp;
}

void HttpConn::InitMsg() {
    path_ = "";
    filetype_ = "";
    is_keepalive_ = false;
}

HttpConn::HttpConn(std::shared_ptr<Channel> Channel)
    : channel_(Channel)
    , storage_(GetConf().GetStorage())
    , parse_state_(PARSE_METHOD)
    , pos_(0)
    , is_keepalive_(false)
    , size_(0) {
    handle_parse_[0] = std::bind(&HttpConn::ParseError, this);
    handle_parse_[1] = std::bind(&HttpConn::ParseMethod, this);
    handle_parse_[2] = std::bind(&HttpConn::ParseHeader, this);
    handle_parse_[3] = std::bind(&HttpConn::ParseSuccess, this);
    channel_->SetReadHandler(std::bind(&HttpConn::Parse, this));
}

// 从in_buffer中找到从pos开始的第一个字符
bool HttpConn::Read(string& msg, const string& str) {
    auto next = in_buffer_.find(str, pos_);
    if (string::npos == next)
        return false;
    msg = in_buffer_.substr(pos_, next - pos_);
    pos_ = next + str.length();
    return true;
}

ParseState HttpConn::ParseMethod() {
    string msg;
    if (!Read(msg, " /"))
        return PARSE_ERROR;
    else if ("GET" == msg)
        method_ = METHOD_GET;
    else if ("POST" == msg)
        method_ = METHOD_POST;
    else
        return PARSE_ERROR;
    if (!Read(path_, " "))
        return PARSE_ERROR;

    struct stat sbuf {};
    if (path_.empty()) { // 默认直接访问index.html
        path_ = "index.html";
    }
    path_ = UrlDecode(path_);
    if (stat((storage_ + path_).c_str(), &sbuf) != 0) {
        LOG_WARN << "No such file: " << storage_ + path_;
        parse_state_ = PARSE_ERROR;
        return PARSE_ERROR;
    }
    if (S_ISDIR(sbuf.st_mode)) {
        // 多一个“/”是为了保险
        path_ += "/index.html";
        // 一定要记得这一步，因为如果当前path_代表一个目录，则要将sbuf更新至path_/index.html，只有这样，在获取st_size时才是文件的大小
        stat((storage_ + path_).c_str(), &sbuf);
    }
    LOG_DEBUG << "Method: " << (method_ == METHOD_GET ? "GET" : "OTHER") << ", File Name: " << path_;
    // LOG_DEBUG << "Test";
    size_ = sbuf.st_size;
    if (!Read(msg, "\r\n")) {
        return PARSE_ERROR;
    } else if ("HTTP/1.0" == msg) {
        version_ = HTTP_10;
    } else if ("HTTP/1.1" == msg) {
        version_ = HTTP_11;
    } else {
        return PARSE_ERROR;
    }
    return PARSE_HEADER;
}

ParseState HttpConn::ParseHeader() {
    if (in_buffer_[pos_] == '\r' && in_buffer_[pos_ + 1] == '\n')
        return PARSE_SUCCESS;
    string key, value;
    if (!Read(key, ": "))
        return PARSE_ERROR;
    if (!Read(value, "\r\n"))
        return PARSE_ERROR;
    header_[key] = value;
    return PARSE_HEADER;
}

// TODO 处理POST请求正文部分
// CONTENTSTATE HttpConn::parseContent(){
//    //暂时只处理get,get没有正文部分
//}

ParseState HttpConn::ParseError() {
    LOG_WARN << "Parse error";
    in_buffer_ = "";
    pos_ = 0;
    InitMsg();
    header_.clear();
    channel_->SetRetEvents(EPOLLOUT | EPOLLET);
    channel_->GetLoop().lock()->UpdatePoller(channel_);
    channel_->SetWriteHandler(bind(&HttpConn::HandleError, this, 400, "Bad Request"));
    return PARSE_METHOD;
}

ParseState HttpConn::ParseSuccess() {
    channel_->SetRetEvents(EPOLLOUT | EPOLLET);
    channel_->GetLoop().lock()->UpdatePoller(channel_);
    in_buffer_ = in_buffer_.substr(pos_ + 2);
    pos_ = 0;
    if (HTTP_11 == version_ && header_.find("Connection") != header_.end() && ("Keep-Alive" == header_["Connection"] || "keep-alive" == header_["Connection"]))
        is_keepalive_ = true;
    int dot_pos = path_.find('.');
    if (string::npos == dot_pos)
        filetype_ = Mimetype::GetMime("default");
    else
        filetype_ = Mimetype::GetMime(path_.substr(dot_pos));
    channel_->SetWriteHandler(std::bind(&HttpConn::Send, this));
    header_.clear();
    return PARSE_METHOD;
}

void HttpConn::Parse() {
    bool zero = false;
    int readsum;
    if (GetConf().GetSSL()) {
        readsum = SslReadn(channel_->GetSsl().get(), in_buffer_, zero);
    } else {
        readsum = Readn(channel_->GetFd(), in_buffer_, zero);
    }
    if (readsum < 0 || zero) {
        InitMsg();
        channel_->SetDeleted(true);
        channel_->GetLoop().lock()->AddTimer(channel_, 0);
        return;
        // 读到RST和FIN默认FIN处理方式，这里的话因为我不太清楚读到RST该怎么处理，就一起这样处理好了
    }
    while (in_buffer_.length() && ~in_buffer_.find("\r\n", pos_)) {
        parse_state_ = handle_parse_[parse_state_]();
    }
}

void HttpConn::Send() {
    string out_buffer;
    if (METHOD_POST == method_) {
        //
    } else if (METHOD_GET == method_) {
        if (is_keepalive_) {
            out_buffer = "HTTP/1.1 200 OK\r\n";
            out_buffer += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" + std::to_string(GetConf().GetKeepAlived()) + "\r\n";
            channel_->GetLoop().lock()->AddTimer(channel_, GetConf().GetKeepAlived());
        } else {
            out_buffer = "HTTP/1.0 200 OK\r\n";
            channel_->GetLoop().lock()->AddTimer(channel_, 0);
        }
        out_buffer += "Content-Type: " + filetype_ + "\r\n";
        out_buffer += "Content-Length: " + std::to_string(size_) + "\r\n";
        out_buffer += "\r\n";
        if (!(GetCache().Get(path_, out_buffer))) {
            int src_fd = Open((storage_ + path_).c_str(), O_RDONLY, 0);
            char* src_addr = (char*) mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, src_fd, 0);
            Close(src_fd);
            string context(src_addr, size_);
            out_buffer += context;
            GetCache().Set(path_, context);
            munmap(src_addr, size_);
        }
    }
    const char* buffer = out_buffer.c_str();
    if (GetConf().GetSSL()) {
        if (!SslWriten(channel_->GetSsl().get(), buffer, out_buffer.length()))
            LOG_ERROR << "ssl writen error";
    } else {
        if (!writen(channel_->GetFd(), buffer, out_buffer.length()))
            LOG_WARN << "Writen error!";
    }
    InitMsg();
    channel_->SetRetEvents(EPOLLIN | EPOLLET);
    channel_->GetLoop().lock()->UpdatePoller(channel_);
}

void HttpConn::HandleError(int errornum, const string& msg) { // 暂时统一用400,Bad Request来处理,留接口可在上层修改错误码和错误信息
    string body = "<html><title>出错了:(</title>";
    body += R"(<head><meta http-equiv="Content-Type" content="text/html; charset=utf-8"></head>)";
    body += "<body bgcolor=\"ffffff\">";
    body += std::to_string(errornum) + msg;
    body += "<hr><em> 400 Bad Request！ </em>\n</body></html>";
    string out_buffer = "HTTP/1.1 " + std::to_string(errornum) + msg + "\r\n";
    out_buffer += "Content-Type: text/html\r\n";
    out_buffer += "Connection: Close\r\n";
    out_buffer += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    ;
    out_buffer += "\r\n";
    out_buffer += body;
    const char* buffer = out_buffer.c_str();
    if (GetConf().GetSSL()) {
        if (!SslWriten(channel_->GetSsl().get(), buffer, out_buffer.length()))
            LOG_ERROR << "ssl writen error";
    } else {
        if (!writen(channel_->GetFd(), buffer, out_buffer.length()))
            LOG_ERROR << "writen error";
    }
    channel_->SetRetEvents(EPOLLIN | EPOLLET);
    channel_->GetLoop().lock()->UpdatePoller(channel_);
}
