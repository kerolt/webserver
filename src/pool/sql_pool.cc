#include "sql_pool.h"

#include <cassert>
#include <semaphore.h>

#include <mutex>

#include <mysql/mysql.h>
#include "log.h"

SqlConnPool* SqlConnPool::Instantce() {
    static SqlConnPool conn_pool;
    return &conn_pool;
}

void SqlConnPool::Init(const char* host, int port, const char* user, const char* pwd, const char* db_name, int conn_size) {
    assert(conn_size > 0);
    for (int i = 0; i < conn_size; i++) {
        MYSQL* sql{};
        sql = mysql_init(sql);
        if (!sql) {
            LOG_ERROR("MySQL init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, db_name, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySQL connect error!");
        }
        conn_queue_.push(sql);
    }
    max_conn_ = conn_size;
    sem_init(&sem_, 0, max_conn_);
}

void SqlConnPool::Close() {
    std::lock_guard lock{mutex_};
    while (!conn_queue_.empty()) {
        auto conn = conn_queue_.front();
        conn_queue_.pop();
        mysql_close(conn);
    }
    mysql_library_end();
}

void SqlConnPool::FreeConn(MYSQL* conn) {
    assert(conn);
    std::lock_guard lock{mutex_};
    conn_queue_.push(conn);
    sem_post(&sem_);
}

int SqlConnPool::GetFreeConnCount() {
    std::lock_guard lock{mutex_};
    return conn_queue_.size();
}

MYSQL* SqlConnPool::GetConn() {
    MYSQL* conn{};
    if (conn_queue_.empty()) {
        LOG_WARN("Sql connection pool busy now!");
        return nullptr;
    }
    sem_wait(&sem_);
    {
        std::lock_guard lock{mutex_};
        conn = conn_queue_.front();
        conn_queue_.pop();
    }
    return conn;
}
