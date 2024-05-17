#include "LogFile.h"

#include <utility>

LogFile::LogFile(std::string Basename, int FlushEveryN)
    : basename(std::move(Basename))
    , flushEveryN(FlushEveryN)
    , count(0)
    , file(NewElement<FileUtil>(basename), DeleteElement<FileUtil>) {
}

void LogFile::Append(const char* logline, int len) {
    AppendUnlocked(logline, len);
}

void LogFile::Flush() {
    file->Flush();
}

bool LogFile::rollFile() {
}

void LogFile::AppendUnlocked(const char* logline, int len) {
    file->Append(logline, len);
    ++count;
    if (flushEveryN == count) {
        count = 0;
        file->Flush();
    }
}
