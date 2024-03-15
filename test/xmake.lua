target("test_log")
    set_kind("binary")
    add_files("test_log.cc", "../src/log/**.cc", "../src/buffer/**.cc")

target("test_timer_heap")
    set_kind("binary")
    add_files("test_timer_heap.cc", "../src/timer/timer_heap.cc")

target("test_thread_pool")
    set_kind("binary")
    add_files("test_thread_pool.cc")

target("test_sql_pool")
    set_kind("binary")
    add_links("mysqlclient")
    add_files("test_sql_pool.cc", "../src/pool/sql_pool.cc", "../src/log/log.cc","../src/buffer/**.cc")