#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Stock Visualizer Build Script${NC}"

# Check if deps directory exists, if not create it
if [ ! -d "deps" ]; then
    echo -e "${YELLOW}Creating deps directory...${NC}"
    mkdir deps
fi

# Function to check if a directory exists
check_dependency() {
    if [ ! -d "deps/$1" ]; then
        echo -e "${RED}Missing dependency: $1${NC}"
        echo "Please run the setup script first or manually clone the dependencies"
        return 1
    else
        echo -e "${GREEN}✓ Found: $1${NC}"
        return 0
    fi
}

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"
check_dependency "imgui" || exit 1
check_dependency "implot" || exit 1  
check_dependency "gl3w" || exit 1

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Navigate to build directory
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

# Build
echo -e "${YELLOW}Building...${NC}"
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || { echo -e "${RED}Build failed${NC}"; exit 1; }

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}Executable: ./build/StockVisualizer${NC}"

# Navigate back
cd ..

# Check if data directory exists
if [ ! -d "data" ]; then
    echo -e "${YELLOW}Note: No 'data' directory found. Create one and add your *_features.csv files there.${NC}"
fi