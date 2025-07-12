cmake_minimum_required(VERSION 3.16)
project(OhlcFeaturesCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Create the library from all .cpp files in src/ EXCEPT for main.cpp ---
file(GLOB LIB_SOURCES "src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*/main.cpp$")

add_library(ohlc_features ${LIB_SOURCES})
target_include_directories(ohlc_features PUBLIC include)


# --- Create the executable ---
add_executable(run_feature_extractor src/main.cpp)
target_link_libraries(run_feature_extractor PRIVATE ohlc_features)


# --- Compiler-specific optimization flags ---
if(MSVC)
    target_compile_options(ohlc_features PRIVATE /O2 /arch:AVX2 /fp:fast)
else()
    target_compile_options(ohlc_features PRIVATE -O3 -march=native -mavx2 -mfma -funroll-loops -ffast-math)
endif()


# --- Parallelism Backend (TBB on Apple) ---
# On Apple systems, the default clang needs TBB to support std::execution::par.
# The "-fopenmp" flag is NOT supported by Apple's Clang.
if(APPLE)
    find_package(TBB REQUIRED)
    message(STATUS "Apple system detected. Linking against TBB for parallel algorithms.")
    # Link the executable to TBB
    target_link_libraries(run_feature_extractor PRIVATE TBB::tbb)
endif()

# For Linux systems with older GCC that might need it
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    find_package(TBB REQUIRED)
    message(STATUS "GCC < 9 detected. Linking against TBB for parallel algorithms.")
    target_link_libraries(run_feature_extractor PRIVATE TBB::tbb)
endif()


# --- Installation ---
install(TARGETS run_feature_extractor DESTINATION bin)