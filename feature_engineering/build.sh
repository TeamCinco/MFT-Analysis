#!/bin/bash

echo "High-Performance OHLCV Feature Extraction - Direct Build"
echo "========================================================"

# Clean previous build
rm -f *.o ohlc_features_cpp

# Fix include paths in source files
echo "Fixing include paths..."

# Fix batch_processor.cpp
sed -i '' 's|#include "../include/batch_processor.h"|#include "batch_processor.h"|g' src/batch_processor.cpp
sed -i '' 's|#include "../include/technical_indicators.h"|#include "technical_indicators.h"|g' src/batch_processor.cpp

# Fix technical_indicators.cpp
sed -i '' 's|#include "../include/technical_indicators.h"|#include "technical_indicators.h"|g' src/technical_indicators.cpp
sed -i '' 's|#include "../include/simd_utils.h"|#include "simd_utils.h"|g' src/technical_indicators.cpp

# Fix simd_utils.cpp
sed -i '' 's|#include "../include/simd_utils.h"|#include "simd_utils.h"|g' src/simd_utils.cpp

# Fix csv_writer.cpp
sed -i '' 's|#include "../include/csv_writer.h"|#include "csv_writer.h"|g' src/csv_writer.cpp

echo "Include paths fixed!"

# Compile
CXX_FLAGS="-std=c++20 -O3 -march=native -mtune=native -ffast-math -DNDEBUG -funroll-loops"
INCLUDES="-Iinclude"
SOURCES="src/main.cpp src/csv_reader.cpp src/batch_processor.cpp src/technical_indicators.cpp src/simd_utils.cpp src/csv_writer.cpp"
LIBS="-pthread"

if [[ $(uname -m) == "arm64" ]]; then
    CXX_FLAGS="$CXX_FLAGS -DAPPLE_SILICON=1"
fi

echo "Compiling..."
clang++ $CXX_FLAGS $INCLUDES $SOURCES $LIBS -o ohlc_features_cpp

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "Running test..."
    ./ohlc_features_cpp /Users/jazzhashzzz/Desktop/Cinco-Quant/00_raw_data/7.9.25 /Users/jazzhashzzz/Desktop/MFT-Analysis/results/7.11.25
else
    echo "❌ Build failed!"
fi