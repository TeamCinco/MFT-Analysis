# MFT Analysis Visualization Suite

A comprehensive visualization suite for financial market analysis, featuring both individual stock analysis and cointegration pairs trading analysis.

## Quick Start

### Build All Visualizers
```bash
./build_all.sh
```

### Run the Unified Launcher (Recommended)
```bash
./build/MFTAnalysis
```

## Available Visualizers

### 1. MFTAnalysis (Unified Launcher) - **RECOMMENDED**
**File:** `./build/MFTAnalysis`

A unified interface that provides access to both stock analysis and pairs analysis in one application. Features:
- Home screen with analysis type selection
- Seamless switching between analysis modes
- Menu-based navigation
- Professional interface

**Usage:**
```bash
./build/MFTAnalysis
```

### 2. StockVisualizer (Individual Stock Analysis)
**File:** `./build/StockVisualizer`

Analyzes individual stocks with 50+ technical indicators and features:
- Technical indicators (RSI, SMA, momentum, etc.)
- Price charts and candlestick patterns
- Volume analysis
- Statistical measures
- Interactive charts and filtering

**Usage:**
```bash
./build/StockVisualizer
```

### 3. PairsVisualizer (Cointegration Analysis)
**File:** `./build/PairsVisualizer`

Specialized tool for pairs trading and statistical arbitrage:
- Cointegration analysis
- Mean reversion detection
- Risk-return optimization
- Outlier identification
- Trading opportunity ranking

**Usage:**
```bash
./build/PairsVisualizer
```

## Build Options

### Build All (Recommended)
```bash
./build_all.sh
```

### Build Individual Components
```bash
# Original stock visualizer
mkdir -p build && cd build
cmake ..
make StockVisualizer

# Pairs visualizer only
./build_pairs.sh

# Unified launcher
mkdir -p build && cd build
cmake ..
make MFTAnalysis
```

## Data Requirements

### Stock Data
- CSV files in `data/` directory
- Format: `SYMBOL_features.csv`
- Contains OHLCV data plus 50+ technical indicators

### Pairs Data
- CSV file with cointegration analysis results
- Real data: `cointegration_results.csv` (73,000+ pairs from your arbitrage analysis)
- Sample file: `cointegration_sample.csv` (2 pairs for testing)
- Contains statistical tests, trading metrics, and performance data

## Features Comparison

| Feature | StockVisualizer | PairsVisualizer | MFTAnalysis |
|---------|----------------|-----------------|-------------|
| Individual Stock Analysis | ✅ | ❌ | ✅ |
| Pairs Trading Analysis | ❌ | ✅ | ✅ |
| Unified Interface | ❌ | ❌ | ✅ |
| Menu Navigation | ❌ | ❌ | ✅ |
| Data Auto-loading | Manual | Auto | Auto |
| Recommended for | Stock research | Pairs trading | All users |

## Documentation

- **Pairs Analysis:** See `README_PAIRS.md` for detailed pairs trading documentation
- **Modular System:** See `README_MODULAR.md` for technical architecture details
- **Integration:** See `INTEGRATION_GUIDE.md` for development information

## Troubleshooting

### Build Issues
```bash
# Install dependencies (macOS)
brew install glfw3 pkg-config

# Clean build
rm -rf build && ./build_all.sh
```

### Runtime Issues
- Ensure data files are in correct directories
- Check OpenGL driver compatibility
- Verify GLFW3 installation

---

**Choose your analysis tool and start exploring the markets!**
