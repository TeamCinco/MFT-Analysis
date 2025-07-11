cmake_minimum_required(VERSION 3.15)
project(ohlc_features VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Aggressive optimization flags for release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -ffast-math -DNDEBUG -funroll-loops")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra -Wpedantic")
endif()

# SIMD detection
include(CheckCXXCompilerFlag)
if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    message(STATUS "Apple Silicon detected - using ARM NEON")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
        message(STATUS "AVX2 support detected")
    endif()
endif()

# Find packages
find_package(Threads REQUIRED)

# Core library sources
set(CORE_SOURCES
    src/batch_processor.cpp
    src/technical_indicators.cpp
    src/simd_utils.cpp
    src/csv_reader.cpp
    src/csv_writer.cpp
)

set(CORE_HEADERS
    include/batch_processor.h
    include/technical_indicators.h
    include/simd_utils.h
    include/csv_reader.h
    include/csv_writer.h
)

# C++ executable only (no Python bindings)
add_executable(ohlc_features_cpp 
    src/main.cpp
    ${CORE_SOURCES}
)

target_include_directories(ohlc_features_cpp PRIVATE include)
target_link_libraries(ohlc_features_cpp PRIVATE Threads::Threads)

# Enable LTO for release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ohlc_features_cpp PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()