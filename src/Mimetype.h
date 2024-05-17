#pragma once

#include <pthread.h>

#include <map>
#include <string>

class Mimetype{
public:
	static std::string GetMime(const std::string &suffix);

private:
    Mimetype();
    static void Init();

    static pthread_once_t once_control_;
    static std::map<std::string,std::string> mime_;
};
