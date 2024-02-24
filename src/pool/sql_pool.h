#ifndef SQL_POOL_H_
#define SQL_POOL_H_

#include <cassert>
#include <semaphore.h>

#include <mutex>
#include <queue>

#include <mysql/mysql.h>
#include "log.h"

class SqlConnPool {
public:
    static SqlConnPool* Instantce();

    void Init(const char* host,
              int port,
              const char* user,
              const char* pwd,
              const char* db_name,
              int conn_size = 10);
    void Close();
    void FreeConn(MYSQL* conn);
    int GetFreeConnCount();
    MYSQL* GetConn();

private:
    SqlConnPool()
        : use_count_(0)
        , free_count_(0) {}

    ~SqlConnPool() {
        Close();
    }

private:
    int max_conn_;
    int use_count_;
    int free_count_;

    std::queue<MYSQL*> conn_queue_;
    std::mutex mutex_;
    sem_t sem_;
};

class SqlConnRAII {
public:
    SqlConnRAII(MYSQL** sql, SqlConnPool* conn_pool) {
        assert(conn_pool);
        *sql = conn_pool->GetConn();
        sql_ = *sql;
        conn_pool_ = conn_pool;
    }

    ~SqlConnRAII() {
        if (sql_) {
            conn_pool_->FreeConn(sql_);
        }
    }

private:
    MYSQL* sql_;
    SqlConnPool* conn_pool_;
};

#endif /* SQL_POOL_H_ */
