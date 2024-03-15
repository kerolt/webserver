#include <unistd.h>
#include <iostream>
#include <string>
#include "sql_pool.h"
#include "log.h"

int main() {
    const char* host = "localhost";
    int port = 3306;
    const char* user = "kerolt";
    const char* pwd = "kerolt";
    const char* db_name = "tiny_server";
    int conn_size = 5;

    Log::Instance()->Init("./", ".log", 1024, DEBUG);

    SqlConnPool* conn_pool = SqlConnPool::Instantce();
    if (conn_pool) {
        try {
            conn_pool->Init(host, port, user, pwd, db_name, conn_size);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize connection pool: %s", e.what());
            return 1;
        }

        // 使用RAII类来管理数据库连接
        MYSQL* sql = nullptr;
        SqlConnRAII connRAII(&sql, conn_pool);

        // 执行SQL查询
        if (sql) {
            // 这里只是一个示例，实际中你需要构造你的SQL查询
            if (mysql_query(sql, "select * from user")) {
                LOG_ERROR("SQL query failed: %s", mysql_error(sql));
            } else {
                std::cout << "Query successful!" << std::endl;
            }
        } else {
            LOG_WARN("No available connections in the pool.");
        }

        // SqlConnRAII析构时会自动释放连接
    } else {
        LOG_ERROR("Failed to get connection pool instance.");
        return 1;
    }

    sleep(1);

    conn_pool->Close();
    std::cout << "Connection pool closed." << std::endl;

    return 0;
}