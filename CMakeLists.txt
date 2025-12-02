cmake_minimum_required(VERSION 3.10)
project(BacktestEngineExample CXX)

# ================================
# Language
# ================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================
# Build type
# ================================
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)"
      FORCE)
endif()

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

# ================================
# Compiler warnings
# ================================
add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wconversion
  -Wsign-conversion
)

# ================================
# Optimization flags
# ================================
set(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native -fomit-frame-pointer"
)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -march=native -mtune=native"
)

# ================================
# Include directories
# ================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ================================
# Dependencies
# ================================
find_package(CURL REQUIRED)

# ================================
# Strategies
# ================================
file(GLOB STRATEGY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/strategies/*.cpp"
)

# ================================
# Executable
# ================================
add_executable(backtest_engine
  src/main.cpp
  src/BacktestEngine.cpp
  src/Portfolio.cpp
  src/SimpleExecutionEngine.cpp
  src/Metrics.cpp
  src/AlphaVantageFeed.cpp
  src/StrategyFactory.cpp

  ${STRATEGY_SOURCES}
)

target_link_libraries(backtest_engine PRIVATE CURL::libcurl)
