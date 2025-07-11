#!/bin/bash
echo "High-Performance OHLCV Feature Extraction"
echo "========================================="

# Clean previous build
rm -f *.o ohlc_features_cpp *.prof 2>/dev/null

echo "Building safe version..."

# Use safer compilation flags
CXX_FLAGS="-std=c++20 -O1 -g -fno-omit-frame-pointer"
INCLUDES="-Iinclude"
LIBS="-pthread"

if [[ $(uname -m) == "arm64" ]]; then
    CXX_FLAGS="$CXX_FLAGS -DAPPLE_SILICON=1"
fi

SOURCES="src/main.cpp src/csv_reader.cpp src/batch_processor.cpp src/technical_indicators.cpp src/simd_utils.cpp src/csv_writer.cpp"

echo "Compiling..."
clang++ $CXX_FLAGS $INCLUDES $SOURCES $LIBS -o ohlc_features_cpp

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "Running..."
    
    ./ohlc_features_cpp /Users/jazzhashzzz/Desktop/Cinco-Quant/00_raw_data/7.11.25 /Users/jazzhashzzz/Desktop/MFT-Analysis/results/7.11.25
else
    echo "❌ Build failed!"
fi