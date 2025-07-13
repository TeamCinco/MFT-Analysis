# Cross-Platform Path Configuration

## What This Is

The project has hardcoded Mac paths that break on Windows. This fixes that.

## Files Changed

- `feature_engineering/src/main.cpp` - hardcoded paths replaced with comment/uncomment system
- `visualization/data` - hardcoded path replaced with comment/uncomment system  
- `arbitrage/config/default_config.yaml` - already works, uses relative paths

## Current Setup

Windows paths are active. Mac paths are commented out.

**Windows (active):**
```
Input:  C:\Users\cinco\Desktop\Cinco-Quant\00_raw_data\7.13
Output: C:\Users\cinco\Desktop\MFT-Analysis\results\7.13
```

**Mac (commented out):**
```
Input:  /Users/jazzhashzzz/Desktop/Cinco-Quant/00_raw_data/7.13
Output: /Users/jazzhashzzz/Desktop/MFT-Analysis/results/7.13
```

## How to Switch Platforms

### Switch to Mac:
1. Open `feature_engineering/src/main.cpp`
2. Comment out Windows lines (add `//`)
3. Uncomment Mac lines (remove `//`)
4. Open `visualization/data`
5. Comment out Windows line (add `#`)
6. Uncomment Mac line (remove `#`)

### Switch to Windows:
1. Open `feature_engineering/src/main.cpp`
2. Comment out Mac lines (add `//`)
3. Uncomment Windows lines (remove `//`)
4. Open `visualization/data`
5. Comment out Mac line (add `#`)
6. Uncomment Windows line (remove `#`)

## Data Flow

Raw data → Feature Engineering → Results folder → Visualization/Arbitrage

## Don't Change This

The path structure is set up to work. Don't modify it unless you want to break things.
