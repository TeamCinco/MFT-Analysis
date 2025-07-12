# Feature Engineering Module - Code Breakdown

This document provides a detailed breakdown of what each file does in the feature engineering module of the MFT-Analysis project.

## Overview

The feature engineering module is a high-performance C++ engine designed to calculate 64+ technical indicators and features from stock price data. It uses SIMD optimizations (NEON for Apple Silicon, AVX2 for Intel) and multi-threading to achieve maximum computational performance.

---

## Header Files (`include/`)

### Core Data Structures

#### `ohlcv_data.h`
- **Purpose**: Defines fundamental data structures for the entire system
- **Key Components**:
  - `OHLCVData`: Stores raw stock price data (Open, High, Low, Close, Volume) with timestamps
  - `FeatureSet`: Stores all 64+ calculated technical indicators and features
- **What it does**: Provides the foundation data types that all other components use

### File I/O Operations

#### `csv_reader.h`
- **Purpose**: High-performance CSV file reading with memory mapping
- **Key Features**:
  - Memory-mapped file I/O for maximum speed
  - Custom fast string-to-double conversion (`fast_atof`)
  - Timestamp parsing for multiple date formats
- **What it does**: Reads stock price CSV files into `OHLCVData` structures as fast as possible

#### `csv_writer.h`
- **Purpose**: High-performance CSV file writing with buffering
- **Key Features**:
  - Large memory buffers (1MB) for optimal write performance
  - Safe handling of missing/NaN values
  - Writes all 64 features in a single wide-format CSV
- **What it does**: Saves calculated features to CSV files for visualization

### Technical Indicator Engines

#### `technical_indicators.h`
- **Purpose**: Standard scalar implementations of all technical indicators
- **What it does**: 
  - Implements 64+ technical indicators using standard C++ algorithms
  - Serves as the baseline/fallback implementation
  - Includes basic indicators (SMA, RSI) and advanced ones (GARCH, Hurst exponent)

#### `simd_technical_indicators.h`
- **Purpose**: AVX2-optimized implementations for Intel processors
- **Key Features**:
  - Uses 256-bit AVX2 instructions for 4x double precision speedup
  - Vectorized array operations (add, subtract, multiply, divide)
  - Optimized rolling window calculations
- **What it does**: Provides 4x faster calculations on Intel/AMD processors

#### `neon_technical_indicators.h`
- **Purpose**: ARM NEON-optimized implementations for Apple Silicon
- **Key Features**:
  - Uses 128-bit NEON instructions for 2x double precision speedup
  - Cache-optimized memory access patterns
  - Prefetching for better memory bandwidth utilization
- **What it does**: Provides 2x faster calculations on Apple M1/M2 processors

#### `simd_utils.h`
- **Purpose**: Utility functions and helpers for SIMD operations
- **What it does**: Provides common SIMD helper functions used by both AVX2 and NEON implementations

### Data Processing

#### `batch_ohlc_processor.h`
- **Purpose**: High-level batch processing coordinator
- **Key Features**:
  - Intelligent caching system to avoid redundant calculations
  - Automatic selection between NEON, AVX2, and scalar implementations
  - Batch processing of multiple stocks simultaneously
- **What it does**: Orchestrates the calculation of all 64 features for multiple stocks efficiently

### Performance Testing

#### `performance_benchmark.h`
- **Purpose**: Basic performance testing and validation
- **What it does**: Tests and compares performance of scalar vs SIMD implementations

#### `multi_core_benchmark.h`
- **Purpose**: Multi-threaded performance testing
- **What it does**: Tests optimal distribution of work across CPU cores (1000 stocks per core)

#### `adaptive_core_benchmark.h`
- **Purpose**: Intelligent core allocation testing
- **What it does**: Dynamically allocates more cores to slower operations and fewer to faster ones

#### `large_scale_benchmark.h`
- **Purpose**: Massive dataset performance testing
- **What it does**: Tests performance on very large datasets (millions of data points)

---

## Source Files (`src/`)

### Main Application

#### `main.cpp`
- **Purpose**: Entry point and application coordinator
- **Key Features**:
  - Command-line argument parsing for different modes
  - Parallel file reading using multiple threads
  - Progress tracking and performance metrics
  - Comprehensive error handling
- **What it does**: 
  - Reads thousands of stock CSV files in parallel
  - Calculates all 64 features for each stock
  - Writes results to output CSV files
  - Reports detailed performance metrics (GFLOPS, throughput)

### File I/O Implementation

#### `csv_reader.cpp`
- **Purpose**: Implementation of high-speed CSV reading
- **Key Optimizations**:
  - Memory-mapped file I/O (mmap) for zero-copy reading
  - Custom `fast_atof` function that's 3x faster than `std::stof`
  - Direct memory parsing without string allocations
- **What it does**: Reads CSV files at maximum I/O bandwidth

#### `csv_writer.cpp`
- **Purpose**: Implementation of high-speed CSV writing
- **Key Optimizations**:
  - Pre-allocates large string buffers to avoid reallocations
  - Single write operation per file for maximum throughput
  - Safe handling of all 64 feature columns with proper offsets
- **What it does**: Writes feature CSV files as fast as possible

### Technical Indicator Implementations

#### `technical_indicators.cpp`
- **Purpose**: Scalar implementations of all 64+ technical indicators
- **Key Categories**:
  - **Basic Indicators**: Returns, SMA, RSI, Volatility, Momentum
  - **Advanced Technical**: KAMA, Linear Slopes, Parkinson Volatility
  - **Statistical Measures**: Z-Score, Percentile Rank, Hurst Exponent
  - **Oscillators**: Chande Momentum, Aroon, TRIX, Fisher Transform
  - **Volume Analysis**: VWAP, Volume Profile, Money Flow Index
  - **Risk Metrics**: CVaR, Sortino Ratio, Ulcer Index, Drawdown Duration
  - **Regime Detection**: Markov Switching, HMM, Structural Breaks
- **What it does**: Provides the mathematical foundation for all feature calculations

#### `simd_technical_indicators.cpp`
- **Purpose**: AVX2-optimized implementations
- **Key Optimizations**:
  - Processes 4 doubles simultaneously using 256-bit registers
  - Vectorized array operations with proper handling of edge cases
  - Optimized rolling window calculations using SIMD
- **What it does**: Accelerates calculations by 4x on Intel/AMD processors

#### `neon_technical_indicators.cpp`
- **Purpose**: ARM NEON-optimized implementations
- **Key Optimizations**:
  - Processes 2 doubles simultaneously using 128-bit registers
  - Cache-friendly memory access patterns with prefetching
  - Fused multiply-add operations for better accuracy and speed
- **What it does**: Accelerates calculations by 2x on Apple Silicon processors

#### `simd_utils.cpp`
- **Purpose**: Common SIMD utility functions
- **What it does**: Provides shared helper functions for SIMD implementations

### Data Processing Implementation

#### `batch_ohlcv_processor.cpp`
- **Purpose**: High-level feature calculation coordinator
- **Key Features**:
  - **Smart Caching**: Avoids recalculating intermediate results
  - **Auto-Detection**: Automatically chooses best implementation (NEON > AVX2 > Scalar)
  - **Dependency Management**: Calculates features in correct order
  - **Memory Efficiency**: Reuses memory allocations where possible
- **What it does**: Efficiently calculates all 64 features for any stock dataset

### Performance Testing Implementation

#### `performance_benchmark.cpp`
- **Purpose**: Comprehensive performance testing suite
- **Key Tests**:
  - Individual indicator performance comparison
  - Memory bandwidth testing
  - Parallel processing efficiency
  - GFLOPS calculations for different data sizes
- **What it does**: Validates that optimizations are working and measures actual speedups

#### `multi_core_benchmark.cpp`
- **Purpose**: Multi-threaded performance optimization
- **Key Features**:
  - Tests different core allocation strategies
  - Measures scaling efficiency across cores
  - Finds optimal work distribution (1000 stocks per core)
- **What it does**: Optimizes parallel processing for maximum throughput

#### `adaptive_core_benchmark.cpp`
- **Purpose**: Intelligent workload distribution
- **Key Features**:
  - Profiles each operation's performance
  - Allocates more cores to slower operations
  - Dynamically balances workload across available cores
- **What it does**: Maximizes overall system performance through smart resource allocation

#### `large_scale_benchmark.cpp`
- **Purpose**: Massive dataset performance testing
- **Key Features**:
  - Tests with millions of data points
  - Measures memory bandwidth utilization
  - Validates performance scaling with data size
- **What it does**: Ensures the system can handle production-scale datasets efficiently

---

## Build Configuration

### `CMakeLists.txt`
- **Purpose**: CMake build configuration
- **Key Features**:
  - Automatic detection of SIMD capabilities (AVX2, NEON)
  - Optimization flags for maximum performance (-O3, -march=native)
  - Proper linking of required libraries
- **What it does**: Configures the build system for optimal performance on the target hardware

### `build.sh`
- **Purpose**: Automated build script
- **Key Features**:
  - Creates build directory
  - Runs CMake configuration
  - Compiles with optimal thread count
  - Provides option to run immediately after building
- **What it does**: Simplifies the build process with a single command

---

## Performance Characteristics

### Computational Performance
- **Baseline (Scalar)**: ~0.077 GFLOPS
- **NEON Optimized**: ~0.15 GFLOPS (2x speedup)
- **AVX2 Optimized**: ~0.30 GFLOPS (4x speedup)
- **Multi-threaded**: Scales linearly with core count

### Throughput Metrics
- **File Reading**: 500+ files/second
- **Feature Calculation**: 100+ stocks/second
- **Overall Processing**: 50+ stocks/second (end-to-end)

### Memory Efficiency
- **Memory-mapped I/O**: Zero-copy file reading
- **SIMD Alignment**: Optimized memory access patterns
- **Cache Optimization**: Prefetching and cache-friendly algorithms

---

## Key Technical Innovations

### 1. **Adaptive SIMD Selection**
Automatically chooses the best implementation based on hardware capabilities:
- NEON for Apple Silicon (M1/M2)
- AVX2 for Intel/AMD processors
- Scalar fallback for older hardware

### 2. **Intelligent Caching**
Avoids redundant calculations by caching intermediate results and reusing them across multiple indicators.

### 3. **Parallel File I/O**
Reads multiple CSV files simultaneously using separate threads, maximizing I/O bandwidth.

### 4. **Memory-Mapped Reading**
Uses mmap() for zero-copy file reading, eliminating memory allocation overhead.

### 5. **Optimized Mathematical Algorithms**
Custom implementations of complex indicators (GARCH, Hurst exponent) optimized for both accuracy and speed.

---

## Usage Examples

### Basic Usage
```bash
# Build the system
./build.sh

# Run with default settings
./build/ohlc_features_cpp

# Run performance benchmarks
./build/ohlc_features_cpp --benchmark
./build/ohlc_features_cpp --multi-core
./build/ohlc_features_cpp --adaptive
```

### Performance Testing
```bash
# Test different optimization levels
./build/ohlc_features_cpp --benchmark        # Basic performance test
./build/ohlc_features_cpp --large-benchmark  # Large dataset test
./build/ohlc_features_cpp --multi-core       # Multi-threading test
./build/ohlc_features_cpp --adaptive         # Adaptive core allocation
```

This feature engineering module represents a state-of-the-art implementation of technical analysis calculations, optimized for modern multi-core processors with SIMD capabilities.
