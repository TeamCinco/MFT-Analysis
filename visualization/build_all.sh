#!/bin/bash

echo "Building MFT Analysis Suite..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure and build all targets
cmake ..

echo "Building all visualization tools..."

# Build original stock visualizer
echo "Building StockVisualizer..."
make StockVisualizer

# Build pairs visualizer
echo "Building PairsVisualizer..."
make PairsVisualizer

# Build unified launcher
echo "Building MFTAnalysis (Unified Launcher)..."
make MFTAnalysis

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful!"
    echo ""
    echo "Available executables:"
    echo "  ./build/StockVisualizer     - Individual stock analysis"
    echo "  ./build/PairsVisualizer     - Cointegration pairs analysis"
    echo "  ./build/MFTAnalysis         - Unified launcher (recommended)"
    echo ""
    echo "To run the unified launcher: ./build/MFTAnalysis"
else
    echo "❌ Build failed!"
    exit 1
fi
