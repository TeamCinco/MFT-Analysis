#!/bin/bash

DATA_DIR="/Users/jazzhashzzz/Desktop/Cinco-Quant/00_raw_data/7.9.25"

# Override with command line argument if provided
if [ $# -ge 1 ]; then
    DATA_DIR="$1"
fi

echo "High-Performance OHLCV Feature Extraction"
echo "========================================"
echo "Data directory: $DATA_DIR"
echo ""

# Check if executable exists
if [ ! -f "build/ohlc_features_cpp" ]; then
    echo "❌ Executable not found. Building first..."
    ./build.sh
    if [ $? -ne 0 ]; then
        echo "❌ Build failed!"
        exit 1
    fi
fi

# Check if data directory exists
if [ ! -d "$DATA_DIR" ]; then
    echo "❌ Data directory does not exist: $DATA_DIR"
    echo "Please provide a valid directory path."
    echo "Usage: $0 [data_directory]"
    exit 1
fi

# Run with timing
echo "⚡ Starting feature extraction..."
echo ""

time ./build/ohlc_features_cpp "$DATA_DIR"