#!/bin/bash

# MFT Statistical Arbitrage Analyzer Build Script
# High-performance build configuration for optimal statistical analysis

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print banner
echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}  MFT Statistical Arbitrage Analyzer   ${NC}"
echo -e "${BLUE}  High-Performance Build System        ${NC}"
echo -e "${BLUE}=========================================${NC}"
echo

# Default values
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_AFTER_BUILD=false
ENABLE_EXCEL=false
ENABLE_BENCHMARKS=true
NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            echo -e "${YELLOW}Debug build enabled${NC}"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            echo -e "${YELLOW}Clean build enabled${NC}"
            shift
            ;;
        -r|--run)
            RUN_AFTER_BUILD=true
            echo -e "${YELLOW}Will run after build${NC}"
            shift
            ;;
        -e|--excel)
            ENABLE_EXCEL=true
            echo -e "${YELLOW}Excel export enabled${NC}"
            shift
            ;;
        --no-benchmarks)
            ENABLE_BENCHMARKS=false
            echo -e "${YELLOW}Benchmarks disabled${NC}"
            shift
            ;;
        -j|--jobs)
            NUM_JOBS="$2"
            echo -e "${YELLOW}Using $NUM_JOBS parallel jobs${NC}"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug         Build in debug mode"
            echo "  -c, --clean         Clean build directory first"
            echo "  -r, --run           Run the analyzer after building"
            echo "  -e, --excel         Enable Excel export (requires xlsxwriter)"
            echo "  --no-benchmarks     Disable performance benchmarks"
            echo "  -j, --jobs NUM      Number of parallel build jobs"
            echo "  -h, --help          Show this help message"
            echo
            echo "Examples:"
            echo "  $0                  # Standard release build"
            echo "  $0 -d -c            # Clean debug build"
            echo "  $0 -r               # Build and run"
            echo "  $0 -e -j 8          # Build with Excel support using 8 jobs"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Detect system information
echo -e "${BLUE}=== System Information ===${NC}"
echo "Operating System: $(uname -s)"
echo "Architecture: $(uname -m)"
echo "CPU Cores: $NUM_JOBS"

# Check for required tools
echo -e "${BLUE}=== Checking Dependencies ===${NC}"

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is required but not installed${NC}"
    exit 1
fi
echo -e "${GREEN}✓ CMake found: $(cmake --version | head -n1)${NC}"

if ! command -v make &> /dev/null && ! command -v ninja &> /dev/null; then
    echo -e "${RED}Error: Make or Ninja is required but not installed${NC}"
    exit 1
fi

if command -v ninja &> /dev/null; then
    BUILD_SYSTEM="Ninja"
    GENERATOR="-G Ninja"
    echo -e "${GREEN}✓ Ninja found: $(ninja --version)${NC}"
else
    BUILD_SYSTEM="Make"
    GENERATOR=""
    echo -e "${GREEN}✓ Make found: $(make --version | head -n1)${NC}"
fi

# Check compiler
if command -v clang++ &> /dev/null; then
    COMPILER="Clang"
    export CXX=clang++
    export CC=clang
    echo -e "${GREEN}✓ Clang found: $(clang++ --version | head -n1)${NC}"
elif command -v g++ &> /dev/null; then
    COMPILER="GCC"
    export CXX=g++
    export CC=gcc
    echo -e "${GREEN}✓ GCC found: $(g++ --version | head -n1)${NC}"
else
    echo -e "${RED}Error: No suitable C++ compiler found${NC}"
    exit 1
fi

# Check for SIMD support
echo -e "${BLUE}=== SIMD Support Detection ===${NC}"
if [[ $(uname -m) == "arm64" ]] || [[ $(uname -m) == "aarch64" ]]; then
    echo -e "${GREEN}✓ ARM NEON support detected${NC}"
elif grep -q avx2 /proc/cpuinfo 2>/dev/null; then
    echo -e "${GREEN}✓ Intel AVX2 support detected${NC}"
elif sysctl -n machdep.cpu.features 2>/dev/null | grep -q AVX2; then
    echo -e "${GREEN}✓ Intel AVX2 support detected${NC}"
else
    echo -e "${YELLOW}⚠ No advanced SIMD support detected (will use scalar fallback)${NC}"
fi

# Create and enter build directory
BUILD_DIR="build"
if [[ $CLEAN_BUILD == true ]] && [[ -d $BUILD_DIR ]]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf $BUILD_DIR
fi

mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure CMake
echo -e "${BLUE}=== Configuring Build ===${NC}"
CMAKE_ARGS=(
    $GENERATOR
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    -DENABLE_EXCEL_EXPORT=$ENABLE_EXCEL
    -DENABLE_BENCHMARKS=$ENABLE_BENCHMARKS
    -DENABLE_JSON_EXPORT=ON
)

echo "CMake configuration:"
echo "  Build type: $BUILD_TYPE"
echo "  Build system: $BUILD_SYSTEM"
echo "  Compiler: $COMPILER"
echo "  Excel export: $ENABLE_EXCEL"
echo "  Benchmarks: $ENABLE_BENCHMARKS"
echo "  Parallel jobs: $NUM_JOBS"

if ! cmake "${CMAKE_ARGS[@]}" ..; then
    echo -e "${RED}CMake configuration failed${NC}"
    exit 1
fi

echo -e "${GREEN}✓ CMake configuration successful${NC}"

# Build the project
echo -e "${BLUE}=== Building Project ===${NC}"
BUILD_START=$(date +%s)

if [[ $BUILD_SYSTEM == "Ninja" ]]; then
    if ! ninja -j $NUM_JOBS; then
        echo -e "${RED}Build failed${NC}"
        exit 1
    fi
else
    if ! make -j $NUM_JOBS; then
        echo -e "${RED}Build failed${NC}"
        exit 1
    fi
fi

BUILD_END=$(date +%s)
BUILD_TIME=$((BUILD_END - BUILD_START))

echo -e "${GREEN}✓ Build completed successfully in ${BUILD_TIME}s${NC}"

# Check if executable was created
if [[ ! -f "arbitrage_analyzer" ]]; then
    echo -e "${RED}Error: Executable not found${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Executable created: arbitrage_analyzer${NC}"

# Create symbolic link in parent directory for convenience
cd ..
if [[ -L "arbitrage_analyzer" ]]; then
    rm arbitrage_analyzer
fi
ln -s build/arbitrage_analyzer arbitrage_analyzer
echo -e "${GREEN}✓ Symbolic link created: ./arbitrage_analyzer${NC}"

# Set up data directories
echo -e "${BLUE}=== Setting Up Data Directories ===${NC}"
mkdir -p data/input data/output

# Create default configuration if it doesn't exist
if [[ ! -f "config/default_config.yaml" ]]; then
    mkdir -p config
    cat > config/default_config.yaml << EOF
# MFT Statistical Arbitrage Analyzer Configuration

# Data source settings
input_data_directory: "../visualization/data/"
output_directory: "data/output/"

# Analysis parameters
min_correlation_threshold: 0.7
max_cointegration_pvalue: 0.05
min_data_points: 100
require_same_sector: false

# Performance settings
num_threads: 0  # 0 = auto-detect
enable_simd: true
enable_caching: true

# Output settings
export_excel: true
export_csv: true
export_json: false
output_filename: "statistical_arbitrage_opportunities"

# Portfolio constraints
max_capital: 5000.0
max_positions: 5
min_stock_price: 10.0
max_stock_price: 500.0
max_position_risk: 0.20
min_expected_return: 0.05
EOF
    echo -e "${GREEN}✓ Default configuration created${NC}"
fi

# Print build summary
echo
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN}  Build Summary                         ${NC}"
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN}Status: SUCCESS${NC}"
echo -e "${GREEN}Build time: ${BUILD_TIME}s${NC}"
echo -e "${GREEN}Executable: ./arbitrage_analyzer${NC}"
echo -e "${GREEN}Configuration: ./config/default_config.yaml${NC}"
echo -e "${GREEN}Data directories: ./data/input, ./data/output${NC}"
echo

# Performance expectations
echo -e "${BLUE}=== Performance Expectations ===${NC}"
if [[ $(uname -m) == "arm64" ]] || [[ $(uname -m) == "aarch64" ]]; then
    echo "Expected SIMD speedup: 2x (NEON)"
elif grep -q avx2 /proc/cpuinfo 2>/dev/null || sysctl -n machdep.cpu.features 2>/dev/null | grep -q AVX2; then
    echo "Expected SIMD speedup: 4x (AVX2)"
else
    echo "Expected SIMD speedup: 1x (scalar)"
fi
echo "Expected multi-threading speedup: ${NUM_JOBS}x"
echo "Estimated analysis rate: 50-200 pairs/second"
echo "Memory usage: ~100MB for 200 stocks"

# Usage instructions
echo
echo -e "${BLUE}=== Usage Instructions ===${NC}"
echo "Run the analyzer:"
echo "  ./arbitrage_analyzer                    # Use default settings"
echo "  ./arbitrage_analyzer --help             # Show all options"
echo "  ./arbitrage_analyzer --benchmark        # Run performance tests"
echo "  ./arbitrage_analyzer --interactive      # Interactive configuration"
echo
echo "Input data should be placed in: ../visualization/data/"
echo "Results will be saved to: ./data/output/"

# Run if requested
if [[ $RUN_AFTER_BUILD == true ]]; then
    echo -e "${BLUE}=== Running Arbitrage Analyzer ===${NC}"
    echo
    ./arbitrage_analyzer
fi

echo -e "${GREEN}Build script completed successfully!${NC}"
