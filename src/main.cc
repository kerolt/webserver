#include "config.h"
#include "server.h"

int main() {
    Config config("./config.json");
    WebServer server(config);
    server.Run();
}
