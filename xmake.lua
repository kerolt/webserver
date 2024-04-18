add_rules("mode.release", "mode.debug")

set_languages("c++17")

add_requires("nlohmann_json")

add_includedirs("src/buffer")
add_includedirs("src/log")
add_includedirs("src/timer")
add_includedirs("src/pool")
add_includedirs("src/server")
add_includedirs("src/http")

target("server")
    set_targetdir("./")
    set_kind("binary")
    add_files("src/**.cc")
    add_links("mysqlclient")
    add_packages("nlohmann_json")

includes("test")