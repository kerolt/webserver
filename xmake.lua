add_rules("mode.release", "mode.debug")

set_languages("c++17")

add_includedirs("src/buffer")
add_includedirs("src/log")
add_includedirs("src/timer")

target("server")
    set_kind("binary")
    add_files("src/**.cc")

target("test_log")
    set_kind("binary")
    add_files("test/test_log.cc", "src/log/**.cc", "src/buffer/**.cc")

target("test_timer_heap")
    set_kind("binary")
    add_files("test/test_timer_heap.cc", "src/timer/timer_heap.cc")