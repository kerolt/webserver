cmake_minimum_required(VERSION 3.10)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

# 添加包含源文件的目录到搜索路径
include_directories(
        src/reactor
        src/log
        src/thread_pool
        src/conf
        src/memory_pool
        src/cache
)

# 定义源文件列表
set(SOURCES
        src/main.cc
        src/Server.cc
        src/HttpConn.cc
        src/Mimetype.cc
        src/Packet.cc
        src/reactor/EventLoop.cc
        src/reactor/Epoll.cc
        src/reactor/Channel.cc
        src/reactor/TimerManager.cc
        src/log/Logging.cc
        src/log/AsyncLogging.cc
        src/log/LogStream.cc
        src/log/LogFile.cc
        src/log/FileUtil.cc
        src/thread_pool/ThreadPoolEventLoop.cc
        src/thread_pool/ThreadEventLoop.cc
        src/thread_pool/Thread.cc
        src/conf/Conf.cc
        src/memory_pool/MemoryPool.cc
        src/cache/LFUCache.cc
)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/web)
set(WEB_DIR_PATH ${PROJECT_SOURCE_DIR}/web)

# 在构建前检测有无web目录，如果有就将web目录删除
if (EXISTS "${WEB_DIR_PATH}")
    message("Web directory exists. Deleting it...")
    file(REMOVE_RECURSE "${WEB_DIR_PATH}")
endif ()

add_executable(webserver ${SOURCES})

target_link_libraries(webserver pthread ssl)

# 添加自定义命令来复制 server.conf 文件和 page 目录到 web 目录
add_custom_command(TARGET webserver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/server.conf
        ${WEB_DIR_PATH}/server.conf
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/page
        ${WEB_DIR_PATH}/page
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${WEB_DIR_PATH}/log
        COMMENT "Copying server.conf and page directory to ./web, and make directory ./web/log"
)