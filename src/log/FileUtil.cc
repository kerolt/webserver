#include "FileUtil.h"

FileUtil::FileUtil(const std::string& filename)
    : fp_(fopen(filename.c_str(), "ae")) {
    setbuffer(fp_, buffer, sizeof(buffer));
}

FileUtil::~FileUtil() {
    fclose(fp_);
}

void FileUtil::Append(const char* log_line, int len) {
    while (len) {
        size_t x = this->Write(log_line, len);
        if (!x) {
            if (ferror(fp_)) {
                perror("FileUtil::Append() failed!");
            }
            break;
        }
        log_line += x;
        len -= x;
    }
}

void FileUtil::Flush() {
    fflush(fp_);
}

int FileUtil::Write(const char* log_line, int len) {
    return fwrite_unlocked(log_line, 1, len, fp_);
}
