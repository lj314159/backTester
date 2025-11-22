cmake_minimum_required(VERSION 3.10)
project(BacktestEngineExample CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Public headers live here.
include_directories(${CMAKE_SOURCE_DIR}/include)

# libcurl for Alpha Vantage HTTP requests.
find_package(CURL REQUIRED)

add_executable(backtest_engine
  src/main.cpp
  src/BacktestEngine.cpp
  src/Portfolio.cpp
  src/SimpleExecutionEngine.cpp
  src/SimpleSMAStrategy.cpp
  src/SmaCrossoverStrategy.cpp
  src/RsiReversionStrategy.cpp
  src/BreakoutStrategy.cpp
  src/VectorMarketDataFeed.cpp
  src/Metrics.cpp
  src/AlphaVantageFeed.cpp
)

# If your CMake/curl package doesn’t provide CURL::libcurl,
# you can replace this with: target_link_libraries(backtest_engine PRIVATE curl)
target_link_libraries(backtest_engine PRIVATE CURL::libcurl)
