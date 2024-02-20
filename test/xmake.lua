target("test_log")
    set_kind("binary")
    add_files("test_log.cc", "../src/log/**.cc", "../src/buffer/**.cc")

target("test_timer_heap")
    set_kind("binary")
    add_files("test_timer_heap.cc", "../src/timer/timer_heap.cc")