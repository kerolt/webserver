#ifndef CONFIG_H_
#define CONFIG_H_

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

struct Config {
    Config() = delete;

    Config(const std::string& json_file) {
        std::ifstream f(json_file);
        nlohmann::json data = nlohmann::json::parse(f);

        port = data["port"];
        trig_mode = data["trigMode"];
        timeout_ms = data["timeoutMs"];
        opt_linger = data["linger"];
        sql_port = data["sqlPort"];
        sql_user = data["sqlUser"];
        sql_pwd = data["sqlPwd"];
        db_name = data["dbName"];
        conn_pool_num = data["connPoolNum"];
        thread_num = data["threadNum"];
        open_log = data["openLog"];
        log_level = data["logLevel"];
        log_que_size = data["logQueSize"];
    }

    int port;
    int trig_mode;
    int timeout_ms;
    bool opt_linger;

    int sql_port;
    std::string sql_user;
    std::string sql_pwd;
    std::string db_name;

    int conn_pool_num;
    int thread_num;

    bool open_log;
    int log_level;
    int log_que_size;
};

#endif /* CONFIG_H_ */
