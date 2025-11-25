cmake_minimum_required(VERSION 3.10)
project(backtester CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# libcurl for Alpha Vantage HTTP
find_package(CURL REQUIRED)

file(GLOB SOURCES
    src/*.cpp
    src/feed/*.cpp
    src/exec/*.cpp
    src/strategies/*.cpp
)

add_executable(backtester ${SOURCES})

target_link_libraries(backtester PRIVATE CURL::libcurl)
