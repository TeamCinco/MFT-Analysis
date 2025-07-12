#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Configuration ---
# The name of the executable target as defined in CMakeLists.txt
EXECUTABLE_NAME="run_feature_extractor"

# --- Script Logic ---
echo "Starting build process..."

# 1. Create a build directory if it doesn't exist and navigate into it.
mkdir -p build
cd build

# 2. Run CMake to configure the project. The ".." points to the parent directory
#    where the root CMakeLists.txt is located.
echo "Configuring with CMake..."
cmake ..

# 3. Run Make to compile the project.
#    The '-j' flag uses multiple cores for a faster build.
#    'nproc' or 'sysctl -n hw.ncpu' gets the number of available cores.
echo "Compiling with Make..."
if [[ "$OSTYPE" == "linux-gnu"* || "$OSTYPE" == "cygwin"* ]]; then
    make -j$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    make -j$(sysctl -n hw.ncpu)
else
    make
fi

echo "Build completed successfully!"
echo "Executable is located at: ./build/$EXECUTABLE_NAME"

# 4. Check if the user wants to run the executable.
if [ "$1" == "run" ]; then
    echo ""
    echo "--- Running Executable ---"
    ./$EXECUTABLE_NAME
    echo "--- Execution Finished ---"
fi