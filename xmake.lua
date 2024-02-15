add_rules("mode.release", "mode.debug")

target("WebServer")
    set_kind("binary")
    set_languages("c++17")
    add_files("src/**.cc")
    add_includedirs("src/buffer")
    add_includedirs("src/log")