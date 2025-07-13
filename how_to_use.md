# How to Install and Run

## Prerequisites

### Windows Installation:

1. **Install Visual Studio Community** (free):
   - Download from https://visualstudio.microsoft.com/vs/community/
   - During installation, select "C++ development" workload
   - This includes MSVC compiler and CMake

2. **Install CMake** (if not included with Visual Studio):
   - Download from https://cmake.org/download/
   - Choose "Windows x64 Installer"
   - During installation, select "Add CMake to system PATH"

3. **Install Git**:
   - Download from https://git-scm.com/download/win
   - Use default installation options

4. **Install Python 3**:
   - Download from https://python.org/downloads/
   - Check "Add Python to PATH" during installation

### Mac Installation:
- Xcode Command Line Tools: `xcode-select --install`
- CMake: `brew install cmake` (if you have Homebrew)
- Python 3: Usually pre-installed or via Homebrew

### Verify Installation (Windows):
Open PowerShell and check:
```powershell
cmake --version
git --version
python --version
```

If any command fails, restart PowerShell and try again. If still failing, reinstall that component.

## Quick Start (Windows)

If you're in the MFT-Analysis folder and CMake is installed:

```powershell
# 1. Feature Engineering
cd feature_engineering
.\build.ps1 -run
cd ..

# 2. Visualization (first time setup)
cd visualization
.\setup.ps1
.\build.ps1
cd ..

# 3. Arbitrage
cd arbitrage
.\build.ps1 -run
```

## Installation Order

Run modules in this order: Feature Engineering → Visualization → Arbitrage

### 1. Feature Engineering Module

Processes raw stock data into feature-engineered datasets.

**Windows (PowerShell):**
```powershell
cd feature_engineering
.\build.ps1
```

To build and run immediately:
```powershell
.\build.ps1 -run
```

**Mac/Linux (Bash):**
```bash
cd feature_engineering
./build.sh
./build.sh run  # To build and run
```

**Executable location:** `./build/run_feature_extractor` (or `.exe` on Windows)

### 2. Visualization Module

**Windows (PowerShell):**
```powershell
cd ..\visualization    # Note: cd .. first if coming from feature_engineering
.\setup.ps1            # First time only
.\build.ps1
```

**Mac/Linux (Bash):**
```bash
cd ../visualization    # Note: cd .. first if coming from feature_engineering
./setup.sh             # First time only
./build.sh
```

**Executable location:** `./build/StockVisualizer` (or `.exe` on Windows)

### 3. Arbitrage Module

**Windows (PowerShell):**
```powershell
cd ..\arbitrage        # Note: cd .. first if coming from visualization
.\build.ps1            # Basic build
.\build.ps1 -run       # Build and run
.\build.ps1 -debug     # Debug build
.\build.ps1 -clean     # Clean build
.\build.ps1 -excel     # Enable Excel export
```

**Mac/Linux (Bash):**
```bash
cd ../arbitrage        # Note: cd .. first if coming from visualization
./build.sh             # Basic build
./build.sh -r          # Build and run
./build.sh -d          # Debug build
./build.sh -c          # Clean build
./build.sh -e          # Enable Excel export
```

**Executable location:** `./build/arbitrage_analyzer` (or `.exe` on Windows)

## Running the Pipeline

### Step 1: Feature Engineering

**Windows:**
```powershell
cd feature_engineering
.\build\run_feature_extractor.exe
```

**Mac/Linux:**
```bash
cd feature_engineering
./build/run_feature_extractor
```

This reads from `C:\Users\cinco\Desktop\Cinco-Quant\00_raw_data\7.13` (Windows) and outputs to `C:\Users\cinco\Desktop\MFT-Analysis\results\7.13`.

### Step 2: Visualization (Optional)

**Windows:**
```powershell
cd visualization
.\build\Release\StockVisualizer.exe  # or .\build\StockVisualizer.exe
```

**Mac/Linux:**
```bash
cd visualization
./build/StockVisualizer
```

This reads the processed data from the results folder.

### Step 3: Arbitrage Analysis

**Windows:**
```powershell
cd arbitrage
.\build\Release\arbitrage_analyzer.exe  # or .\build\arbitrage_analyzer.exe
```

**Mac/Linux:**
```bash
cd arbitrage
./build/arbitrage_analyzer
```

For interactive setup:
```powershell
.\build\Release\arbitrage_analyzer.exe --interactive  # Windows
./build/arbitrage_analyzer --interactive              # Mac/Linux
```

For help:
```powershell
.\build\Release\arbitrage_analyzer.exe --help  # Windows
./build/arbitrage_analyzer --help              # Mac/Linux
```

## Platform Switching

If switching between Mac and Windows, edit these files:

1. `feature_engineering/src/main.cpp` - comment/uncomment the path lines
2. `visualization/data` - comment/uncomment the path line

See `cross-platform-config/README.md` for details.

## Troubleshooting

### Build fails
- Check if all prerequisites are installed
- Try clean build: `./build.sh -c`
- Check error messages for missing dependencies

### Feature engineering can't find data
- Verify the data folder exists: `C:\Users\cinco\Desktop\Cinco-Quant\00_raw_data\7.13`
- Check if paths are correct for your platform in the source files

### Visualization shows no data
- Run feature engineering first
- Check if results folder has the processed CSV files
- Verify `visualization/data` file points to correct path

### Arbitrage analyzer fails
- Check if feature engineering completed successfully
- Verify configuration in `arbitrage/config/default_config.yaml`
- Try: `./arbitrage_analyzer --help`

## Output Locations

- **Feature Engineering**: `C:\Users\cinco\Desktop\MFT-Analysis\results\7.13\`
- **Arbitrage**: `arbitrage/data/output/`

Files will be CSV, Excel, or JSON depending on configuration.
