cmake_minimum_required(VERSION 3.16)
project(ArbitrageAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Detect SIMD capabilities
include(CheckCXXCompilerFlag)

# Check for AVX2 support
check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
if(COMPILER_SUPPORTS_AVX2)
    add_definitions(-DHAVE_AVX2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
    message(STATUS "AVX2 support: ENABLED")
else()
    message(STATUS "AVX2 support: DISABLED")
endif()

# Check for NEON support (ARM)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    add_definitions(-DHAVE_NEON)
    message(STATUS "NEON support: ENABLED")
else()
    message(STATUS "NEON support: DISABLED")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/statistics
    ${CMAKE_CURRENT_SOURCE_DIR}/include/export
)

# Source files
set(CORE_SOURCES
    src/core/stock_data.cpp
    src/core/fast_csv_loader.cpp
    src/core/arbitrage_analyzer.cpp
)

set(STATISTICS_SOURCES
    src/statistics/simd_statistics.cpp
    src/statistics/cointegration_analyzer.cpp
    src/statistics/correlation_analyzer.cpp
)

set(EXPORT_SOURCES
    src/export/excel_exporter.cpp
    src/export/csv_exporter.cpp
    src/export/json_exporter.cpp
)

set(ALL_SOURCES
    ${CORE_SOURCES}
    ${STATISTICS_SOURCES}
    ${EXPORT_SOURCES}
    main.cpp
)

# Create the executable
add_executable(arbitrage_analyzer ${ALL_SOURCES})

# Link libraries
target_link_libraries(arbitrage_analyzer 
    Threads::Threads
)

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    target_link_libraries(arbitrage_analyzer "-framework Accelerate")
    message(STATUS "Platform: macOS - Accelerate framework linked")
elseif(UNIX)
    # Linux specific settings
    target_link_libraries(arbitrage_analyzer m)
    message(STATUS "Platform: Linux")
elseif(WIN32)
    # Windows specific settings
    message(STATUS "Platform: Windows")
endif()

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(arbitrage_analyzer PRIVATE
        -Wall -Wextra -Wpedantic
        -ffast-math
        -funroll-loops
        -ftree-vectorize
    )
    message(STATUS "Compiler: GCC - optimization flags enabled")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(arbitrage_analyzer PRIVATE
        -Wall -Wextra -Wpedantic
        -ffast-math
        -funroll-loops
        -fvectorize
    )
    message(STATUS "Compiler: Clang - optimization flags enabled")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(arbitrage_analyzer PRIVATE
        /W4
        /fp:fast
        /arch:AVX2
    )
    message(STATUS "Compiler: MSVC - optimization flags enabled")
endif()

# Optional: Excel export support (requires xlsxwriter or similar)
option(ENABLE_EXCEL_EXPORT "Enable Excel export functionality" OFF)
if(ENABLE_EXCEL_EXPORT)
    # Try to find xlsxwriter or similar library
    find_library(XLSXWRITER_LIB xlsxwriter)
    if(XLSXWRITER_LIB)
        target_link_libraries(arbitrage_analyzer ${XLSXWRITER_LIB})
        add_definitions(-DHAVE_EXCEL_EXPORT)
        message(STATUS "Excel export: ENABLED")
    else()
        message(WARNING "Excel export requested but xlsxwriter library not found")
        message(STATUS "Excel export: DISABLED (falling back to CSV)")
    endif()
else()
    message(STATUS "Excel export: DISABLED (CSV export will be used)")
endif()

# Optional: JSON export support
option(ENABLE_JSON_EXPORT "Enable JSON export functionality" ON)
if(ENABLE_JSON_EXPORT)
    add_definitions(-DHAVE_JSON_EXPORT)
    message(STATUS "JSON export: ENABLED")
else()
    message(STATUS "JSON export: DISABLED")
endif()

# Performance benchmarking
option(ENABLE_BENCHMARKS "Enable performance benchmarking" ON)
if(ENABLE_BENCHMARKS)
    add_definitions(-DHAVE_BENCHMARKS)
    message(STATUS "Benchmarks: ENABLED")
else()
    message(STATUS "Benchmarks: DISABLED")
endif()

# Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
    message(STATUS "Debug mode: ENABLED")
else()
    message(STATUS "Debug mode: DISABLED")
endif()

# Installation
install(TARGETS arbitrage_analyzer
    RUNTIME DESTINATION bin
)

# Create data directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data/input)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data/output)

# Create configuration directory and file if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config)
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/default_config.yaml)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/config/default_config.yaml
"# MFT Statistical Arbitrage Analyzer Configuration
input_data_directory: \"../visualization/data/\"
output_directory: \"data/output/\"
min_correlation_threshold: 0.7
max_cointegration_pvalue: 0.05
min_data_points: 100
require_same_sector: false
num_threads: 0
enable_simd: true
enable_caching: true
export_excel: true
export_csv: true
export_json: false
output_filename: \"statistical_arbitrage_opportunities\"
max_capital: 5000.0
max_positions: 5
min_stock_price: 10.0
max_stock_price: 500.0
max_position_risk: 0.20
min_expected_return: 0.05
")
endif()

# Copy configuration files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/default_config.yaml
    ${CMAKE_CURRENT_BINARY_DIR}/config/default_config.yaml
    COPYONLY
)

# Print build summary
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Threads: ${CMAKE_THREAD_LIBS_INIT}")
message(STATUS "===================================")
message(STATUS "")

# Performance estimation
message(STATUS "Expected Performance (based on hardware):")
if(COMPILER_SUPPORTS_AVX2)
    message(STATUS "  - SIMD acceleration: 4x speedup (AVX2)")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "  - SIMD acceleration: 2x speedup (NEON)")
else()
    message(STATUS "  - SIMD acceleration: None (scalar fallback)")
endif()

if(DEFINED CMAKE_HOST_SYSTEM_PROCESSOR_COUNT AND CMAKE_HOST_SYSTEM_PROCESSOR_COUNT)
    math(EXPR ESTIMATED_CORES "${CMAKE_HOST_SYSTEM_PROCESSOR_COUNT}")
    message(STATUS "  - Multi-threading: ${ESTIMATED_CORES}x speedup potential")
else()
    message(STATUS "  - Multi-threading: Available")
endif()

message(STATUS "  - Expected analysis rate: 50-200 pairs/second")
message(STATUS "  - Memory usage: ~100MB for 200 stocks")
message(STATUS "")
