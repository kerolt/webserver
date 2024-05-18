#include <csignal>

#include "Server.h"
#include "conf/Conf.h"

int main(int argc, char** argv) {
    InitMemoryPool();
    const char* parm = "hvc:";
    char conf[100] = "./server.conf";
    int opt;
    while (~(opt = getopt(argc, argv, parm))) {
        switch (opt) {
            case 'h':
                printf("Options:\n");
                printf("  -h\t: this help\n");
                printf("  -v\t: show version\n");
                printf("  -c\t: set configuration file(default: ./server.conf)\n)");
                return 0;
            case 'v':
                printf("WebServer version WebServer/1.11\n");
                return 0;
            case 'c':
                strncpy(conf, optarg, 99);
                break;
            default:
                break;
        }
    }

    GetConf().Init(conf);
    GetCache().Init();
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, EventLoop::SetQuit);
    signal(SIGQUIT, EventLoop::SetQuit);

    Server server(GetConf().GetPort().c_str(), GetConf().GetIoThreads());
    std::cout << "Server is running now..." << '\n';
    server.Run();
}
