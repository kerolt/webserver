#include "Mimetype.h"

pthread_once_t Mimetype::once_control_=PTHREAD_ONCE_INIT;
std::map<std::string,std::string> Mimetype::mime_;

void Mimetype::Init(){
    mime_[".html"] = "text/html";
    mime_[".htm"] = "text/html";
    mime_[".css"] = "text/css";
    mime_[".js"] = "text/javascript";
    mime_[".avi"] = "video/x-msvideo";
    mime_[".bmp"] = "image/bmp";
    mime_[".doc"] = "application/msword";
    mime_[".gz"] = "application/x-gzip";
    mime_[".ico"] = "image/x-icon";
    mime_[".jpg"] = "image/jpeg";
    mime_[".gif"] = "image/gif";
    mime_[".png"] = "image/png";
    mime_[".txt"] = "text/plain";
    mime_[".mp3"] = "audio/mp3";
    mime_[".rtf"] = "application/rtf";
    mime_[".pdf"] = "application/pdf";
    mime_["default"] = "text/plain";
}

Mimetype::Mimetype() = default;

std::string Mimetype::GetMime(const std::string &suffix) {
    pthread_once(&once_control_, Mimetype::Init);
	if(mime_.find(suffix) == mime_.end()) {
        return mime_["default"];
    }
    return mime_[suffix];
}
