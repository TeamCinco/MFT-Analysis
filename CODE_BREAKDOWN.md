# MFT-Analysis Code Breakdown

This document provides a simple breakdown of what each file does in the MFT-Analysis project.

## Project Structure Overview

The project consists of two main components:
- **feature_engineering/**: High-performance C++ engine for calculating technical indicators
- **visualization/**: Real-time GUI application for visualizing the calculated features

---

## Feature Engineering Module (`feature_engineering/`)

### Core Data Structures

#### `ohlcv_data.h`
- **Purpose**: Defines the basic OHLCV (Open, High, Low, Close, Volume) data structure
- **What it does**: Contains the fundamental stock price data that all calculations are based on

### File I/O Operations

#### `csv_reader.h` / `csv_reader.cpp`
- **Purpose**: Reads stock data from CSV files
- **What it does**: Parses CSV files containing stock price data and converts them into OHLCV structures

#### `csv_writer.h` / `csv_writer.cpp`
- **Purpose**: Writes calculated features to CSV files
- **What it does**: Takes computed technical indicators and saves them to CSV files for later use

### Technical Indicator Calculations

#### `technical_indicators.h` / `technical_indicators.cpp`
- **Purpose**: Standard technical indicator calculations
- **What it does**: Implements basic indicators like SMA, RSI, volatility, momentum, etc.

#### `simd_technical_indicators.h` / `simd_technical_indicators.cpp`
- **Purpose**: SIMD-optimized technical indicators
- **What it does**: Same calculations as above but using SIMD instructions for faster processing

#### `neon_technical_indicators.h` / `neon_technical_indicators.cpp`
- **Purpose**: ARM NEON-optimized technical indicators
- **What it does**: ARM-specific SIMD optimizations for technical indicators (for Apple Silicon Macs)

#### `simd_utils.h` / `simd_utils.cpp`
- **Purpose**: SIMD utility functions
- **What it does**: Helper functions for SIMD operations used by the optimized indicator calculations

### Data Processing

#### `batch_ohlc_processor.h` / `batch_ohlcv_processor.cpp`
- **Purpose**: Batch processing of multiple stocks
- **What it does**: Processes large numbers of stock files efficiently, calculating all technical indicators

### Performance Testing

#### `performance_benchmark.h` / `performance_benchmark.cpp`
- **Purpose**: Basic performance benchmarking
- **What it does**: Times how long it takes to calculate indicators for performance comparison

#### `multi_core_benchmark.h` / `multi_core_benchmark.cpp`
- **Purpose**: Multi-threaded performance testing
- **What it does**: Tests performance using multiple CPU cores simultaneously

#### `adaptive_core_benchmark.h` / `adaptive_core_benchmark.cpp`
- **Purpose**: Adaptive core usage benchmarking
- **What it does**: Automatically adjusts the number of CPU cores used based on system capabilities

#### `large_scale_benchmark.h` / `large_scale_benchmark.cpp`
- **Purpose**: Large dataset performance testing
- **What it does**: Tests performance on very large datasets to measure scalability

### Main Application

#### `main.cpp`
- **Purpose**: Entry point for the feature engineering application
- **What it does**: Coordinates the entire process of reading data, calculating indicators, and writing results

### Build Configuration

#### `CMakeLists.txt`
- **Purpose**: CMake build configuration
- **What it does**: Defines how to compile the C++ code with proper optimizations and dependencies

#### `build.sh`
- **Purpose**: Build script
- **What it does**: Automates the compilation process

---

## Visualization Module (`visualization/`)

### Core Application

#### `main.cpp`
- **Purpose**: Entry point for the visualization application
- **What it does**: Initializes the GUI framework and starts the main application loop

#### `StockVisualizer.h` / `StockVisualizer.cpp`
- **Purpose**: Main application controller
- **What it does**: Manages the overall GUI application, coordinates data loading and chart rendering

### Data Management

#### `StockData.h`
- **Purpose**: Data structure for visualization
- **What it does**: Defines how stock data and all calculated features are stored in memory

#### `FileManager.h` / `FileManager.cpp`
- **Purpose**: File loading and management
- **What it does**: Loads CSV files with calculated features, handles multiple file formats safely

### Chart Rendering

#### `ChartRenderer.h` / `ChartRenderer.cpp`
- **Purpose**: Chart creation and rendering
- **What it does**: Creates all the different types of charts (price charts, technical indicators, oscillators, etc.)

### User Interface

#### `UIComponents.h` / `UIComponents.cpp`
- **Purpose**: User interface elements
- **What it does**: Creates buttons, dropdowns, progress bars, and other GUI components

### Modular Visualization System

#### `core/ChartFactory.h` / `core/ChartFactory.cpp`
- **Purpose**: Chart creation factory
- **What it does**: Creates different types of charts based on requests

#### `core/FeatureExtractor.h` / `core/FeatureExtractor.cpp`
- **Purpose**: Feature data extraction
- **What it does**: Extracts specific features from the loaded data for visualization

#### `core/FeatureRegistry.h` / `core/FeatureRegistry.cpp`
- **Purpose**: Feature registration system
- **What it does**: Keeps track of all available features and their properties

#### `VisualizationManager.h` / `VisualizationManager.cpp`
- **Purpose**: Visualization coordination
- **What it does**: Manages the overall visualization system and coordinates between components

### Build Configuration

#### `CMakeLists.txt`
- **Purpose**: CMake build configuration for visualization
- **What it does**: Defines how to compile the GUI application with ImGui and ImPlot dependencies

#### `build.sh`
- **Purpose**: Build script for visualization
- **What it does**: Automates the compilation of the visualization application

#### `setup.sh`
- **Purpose**: Dependency setup script
- **What it does**: Downloads and sets up required GUI libraries (ImGui, ImPlot, etc.)

---

## Data Files

### `visualization/data/`
- **Purpose**: Storage for calculated features
- **What it contains**: Thousands of CSV files, each containing 64 calculated features for individual stocks
- **Format**: `SYMBOL_features.csv` (e.g., `AAPL_features.csv`)

---

## Key Features Calculated

The system calculates 64 different features for each stock, including:

### Basic Indicators
- Price data (OHLC), Volume, Returns
- Simple Moving Average (SMA)
- Relative Strength Index (RSI)
- Volatility, Momentum

### Advanced Technical Indicators
- KAMA (Kaufman Adaptive Moving Average)
- Parkinson Volatility
- Linear Slopes (trend analysis)
- Ichimoku Cloud components
- SuperTrend indicator

### Volume Analysis
- Volume-Weighted Average Price (VWAP)
- Volume Profile (High/Low Volume Nodes)
- On-Balance Volume
- Money Flow Index

### Statistical Measures
- Skewness and Kurtosis
- Z-Score and Percentile Ranking
- Hurst Exponent
- GARCH Volatility

### Risk Metrics
- Conditional Value at Risk (CVaR)
- Sortino Ratio
- Ulcer Index
- Drawdown Duration

### Market Regime Analysis
- Markov Regime Switching
- Hidden Markov Model states
- Structural break detection (Chow test)

---

## How It All Works Together

1. **Feature Engineering** reads stock price data and calculates 64 technical indicators
2. **Results are saved** to CSV files in the `visualization/data/` directory
3. **Visualization application** loads these CSV files and creates interactive charts
4. **Users can explore** different stocks and analyze various technical indicators through the GUI

The system is designed for high performance, using SIMD optimizations and multi-threading to process thousands of stocks efficiently.
