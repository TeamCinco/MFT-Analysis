# Cointegration Pairs Visualizer

A specialized visualization tool for analyzing market pairs cointegration data, designed to help traders identify outliers and trading opportunities.

## Features

### 📊 Interactive Visualizations
- **Statistical Significance Plot**: ADF Statistic vs P-Value with significance thresholds
- **Risk vs Return Analysis**: Sharpe Ratio vs Expected Return scatter plots
- **Mean Reversion Analysis**: Half-Life vs Current Z-Score with entry/exit thresholds
- **Distribution Analysis**: Histograms for Sharpe ratios, win rates, and half-life periods

### 🔍 Advanced Filtering
- Filter by cointegration status (only cointegrated pairs)
- Filter by quality grade (A, B, C ratings)
- Filter by outlier status (extreme performance metrics)
- Adjustable Sharpe ratio and half-life thresholds
- Real-time filter application

### 📈 Trading-Focused Analysis
- **Top Opportunities Table**: Best pairs ranked by Sharpe ratio
- **Detailed Pair Analysis**: Complete statistical and trading metrics
- **Outlier Detection**: Automatic identification of exceptional pairs
- **Performance Metrics**: Win rates, historical trades, expected returns

### 💾 Data Management
- CSV file loading and export
- Filtered data export functionality
- Sample data included for testing

## Quick Start

### Building the Application

```bash
# Option 1: Use the build script
./build_pairs.sh

# Option 2: Manual build
mkdir -p build && cd build
cmake ..
make PairsVisualizer
```

### Running the Application

```bash
# From the visualization directory
./build/PairsVisualizer
```

The application will automatically load the sample cointegration data (`cointegration_sample.csv`).

## Data Format

The visualizer expects CSV files with the following columns:

```
Stock1,Stock2,ADF_Statistic,P_Value,Critical_1pct,Critical_5pct,Critical_10pct,
Half_Life_Days,Hedge_Ratio,Spread_Mean,Spread_StdDev,Max_Spread,Min_Spread,
Current_Spread,Z_Score,Grade,Is_Cointegrated,Entry_Threshold,Exit_Threshold,
Expected_Return,Sharpe_Ratio,Historical_Trades,Win_Rate
```

### Sample Data
```csv
ALGN,STAA,-3.665737,0.010000,-3.430000,-2.860000,-2.570000,98.18,0.172327,-1.729117,13.498094,61.777534,-34.328176,-17.461123,-1.165498,C,TRUE,2.000000,0.500000,0.001695,0.202213,35,0.542857
JLS,EBF,-5.369563,0.010000,-3.430000,-2.860000,-2.570000,46.72,-0.406912,27.216953,2.090581,32.668008,20.583561,26.003192,-0.580585,A,TRUE,2.000000,0.500000,0.013807,1.340597,8,0.875000
```

## User Interface

### Dashboard Tab
- **Summary Statistics**: Overview of total pairs, cointegrated pairs, high-quality pairs, and outliers
- **Quick Filters**: Checkboxes and sliders for rapid data filtering
- **Risk vs Return Overview**: Main scatter plot for initial analysis

### Scatter Plots Tab
- **Statistical Significance**: ADF vs P-Value with 5% significance line
- **Risk vs Return**: Sharpe Ratio vs Expected Return
- **Mean Reversion**: Half-Life vs Z-Score with entry/exit thresholds

### Distributions Tab
- **Sharpe Ratio Distribution**: Histogram showing performance distribution
- **Win Rate Distribution**: Success rate across all pairs
- **Half-Life Distribution**: Mean reversion speed analysis

### Top Opportunities Tab
- **Ranked Table**: Top 20 pairs sorted by Sharpe ratio
- **Color-coded Status**: Visual indicators for outliers and high-quality pairs
- **Key Metrics**: All essential trading information in one view

### Pair Details Tab
- **Individual Analysis**: Complete breakdown of selected pair
- **Statistical Tests**: ADF results and critical values
- **Trading Metrics**: Performance and risk measures
- **Spread Analysis**: Current position and historical statistics

## Key Metrics Explained

### Statistical Measures
- **ADF Statistic**: Augmented Dickey-Fuller test statistic (more negative = more stationary)
- **P-Value**: Statistical significance (< 0.05 typically considered significant)
- **Critical Values**: Thresholds for 1%, 5%, and 10% significance levels

### Trading Metrics
- **Half-Life**: Average time for spread to mean-revert (shorter = faster reversion)
- **Z-Score**: Current spread position relative to historical mean
- **Sharpe Ratio**: Risk-adjusted return measure
- **Win Rate**: Percentage of profitable historical trades

### Quality Classifications
- **Grade A**: High-quality pairs with strong statistical properties
- **Grade B**: Medium-quality pairs with acceptable metrics
- **Grade C**: Lower-quality pairs requiring careful evaluation
- **Outliers**: Pairs with exceptional performance metrics (Z-Score > 2.5, Sharpe > 2.0, Win Rate > 80%)

## Trading Interpretation

### Entry Signals
- **High Z-Score (> 2.0)**: Spread significantly above mean (potential short opportunity)
- **Low Z-Score (< -2.0)**: Spread significantly below mean (potential long opportunity)
- **Short Half-Life**: Faster mean reversion (better for short-term trades)

### Quality Indicators
- **High Sharpe Ratio (> 1.0)**: Good risk-adjusted returns
- **High Win Rate (> 60%)**: Consistent profitability
- **Low P-Value (< 0.05)**: Strong statistical cointegration

### Risk Factors
- **Long Half-Life (> 100 days)**: Slow mean reversion
- **High Spread Volatility**: Increased position risk
- **Low Historical Trades**: Limited backtesting data

## File Menu Options

### Load CSV
- Load your own cointegration data files
- Supports standard CSV format with headers
- Automatic data validation and parsing

### Export Filtered Data
- Save current filtered results to CSV
- Maintains all original columns and formatting
- Useful for further analysis or record-keeping

## Technical Requirements

### Dependencies
- OpenGL 3.3+
- GLFW3
- ImGui (included)
- ImPlot (included)
- C++17 compiler

### Supported Platforms
- macOS (tested)
- Linux (should work)
- Windows (should work with minor modifications)

## Troubleshooting

### Build Issues
```bash
# Ensure dependencies are installed (macOS)
brew install glfw3 pkg-config

# Clean build
rm -rf build && mkdir build && cd build && cmake .. && make PairsVisualizer
```

### Runtime Issues
- Ensure `cointegration_sample.csv` is in the visualization directory
- Check that OpenGL drivers are up to date
- Verify GLFW3 is properly installed

## Extending the Visualizer

### Adding New Metrics
1. Update `CointegrationData.h` with new fields
2. Modify CSV parsing in `CointegrationVisualizer.cpp`
3. Add new visualization functions
4. Update filtering logic if needed

### Custom Visualizations
- Use ImPlot functions to create new chart types
- Add new tabs in the `renderUI()` function
- Implement custom analysis functions

## Performance Notes

- Optimized for datasets up to 10,000 pairs
- Real-time filtering and sorting
- Efficient memory usage with smart data structures
- Responsive UI even with large datasets

---

**Built for traders, by traders. Analyze smarter, trade better.**
