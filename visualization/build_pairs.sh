#!/bin/bash

echo "Building Cointegration Pairs Visualizer..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure and build
cmake ..
make PairsVisualizer

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./build/PairsVisualizer"
else
    echo "Build failed!"
    exit 1
fi
