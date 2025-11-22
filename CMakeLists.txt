cmake_minimum_required(VERSION 3.10)
project(BacktestEngineExample CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(CURL REQUIRED)

add_executable(backtest_engine
    src/BacktestEngine.cpp
    src/main.cpp
    src/Portfolio.cpp
    src/SimpleExecutionEngine.cpp
    src/SimpleSMAStrategy.cpp
    src/VectorMarketDataFeed.cpp
    src/Metrics.cpp
    src/AlphaVantageFeed.cpp
)

target_link_libraries(backtest_engine PRIVATE CURL::libcurl)
