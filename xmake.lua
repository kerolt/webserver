add_rules("mode.release", "mode.debug")

set_languages("c++17")

add_includedirs("src/buffer")
add_includedirs("src/log")
add_includedirs("src/timer")

target("server")
    set_kind("binary")
    add_files("src/**.cc")

includes("test")