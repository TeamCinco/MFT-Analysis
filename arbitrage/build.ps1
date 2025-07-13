# PowerShell build script for MFT Statistical Arbitrage Analyzer
param(
    [switch]$debug,
    [switch]$clean,
    [switch]$run,
    [switch]$excel,
    [switch]$help,
    [int]$jobs = [Environment]::ProcessorCount
)

if ($help) {
    Write-Host "Usage: .\build.ps1 [OPTIONS]"
    Write-Host "Options:"
    Write-Host "  -debug          Build in debug mode"
    Write-Host "  -clean          Clean build directory first"
    Write-Host "  -run            Run the analyzer after building"
    Write-Host "  -excel          Enable Excel export"
    Write-Host "  -jobs NUM       Number of parallel build jobs (default: $jobs)"
    Write-Host "  -help           Show this help message"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build.ps1                # Standard release build"
    Write-Host "  .\build.ps1 -debug -clean  # Clean debug build"
    Write-Host "  .\build.ps1 -run           # Build and run"
    exit 0
}

Write-Host "=========================================" -ForegroundColor Blue
Write-Host "  MFT Statistical Arbitrage Analyzer   " -ForegroundColor Blue
Write-Host "  Windows PowerShell Build System      " -ForegroundColor Blue
Write-Host "=========================================" -ForegroundColor Blue

# Find CMake executable
$CMAKE_PATHS = @(
    "cmake",
    "C:\Program Files\CMake\bin\cmake.exe",
    "C:\Program Files (x86)\CMake\bin\cmake.exe",
    "${env:ProgramFiles}\CMake\bin\cmake.exe",
    "${env:ProgramFiles(x86)}\CMake\bin\cmake.exe"
)

$CMAKE_EXE = $null
foreach ($path in $CMAKE_PATHS) {
    try {
        $result = & $path --version 2>$null
        if ($LASTEXITCODE -eq 0) {
            $CMAKE_EXE = $path
            Write-Host "Found CMake: $path" -ForegroundColor Green
            break
        }
    } catch {
        # Continue to next path
    }
}

if (!$CMAKE_EXE) {
    Write-Host "Error: CMake not found. Please install CMake or add it to PATH." -ForegroundColor Red
    Write-Host "Expected locations:" -ForegroundColor Yellow
    foreach ($path in $CMAKE_PATHS) {
        Write-Host "  - $path" -ForegroundColor Yellow
    }
    exit 1
}

# Configuration
$BUILD_TYPE = if ($debug) { "Debug" } else { "Release" }
$ENABLE_EXCEL = $excel.ToString().ToLower()

Write-Host "Build Configuration:" -ForegroundColor Yellow
Write-Host "  Build type: $BUILD_TYPE"
Write-Host "  Excel export: $ENABLE_EXCEL"
Write-Host "  Parallel jobs: $jobs"

# Clean build if requested
if ($clean -and (Test-Path "build")) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
}

# Create build directory
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build"
}

Set-Location "build"

try {
    # Configure with CMake
    Write-Host "Configuring with CMake..." -ForegroundColor Yellow
    & $CMAKE_EXE -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DENABLE_EXCEL_EXPORT=$ENABLE_EXCEL ..
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build
    Write-Host "Building..." -ForegroundColor Yellow
    & $CMAKE_EXE --build . --config $BUILD_TYPE --parallel $jobs
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host "Build completed successfully!" -ForegroundColor Green

    # Check if executable exists
    $executablePath = ".\$BUILD_TYPE\arbitrage_analyzer.exe"
    if (!(Test-Path $executablePath)) {
        $executablePath = ".\arbitrage_analyzer.exe"
    }
    
    if (Test-Path $executablePath) {
        Write-Host "Executable created: $executablePath" -ForegroundColor Green
    } else {
        throw "Executable not found"
    }

    Set-Location ".."

    # Create data directories
    if (!(Test-Path "data")) {
        New-Item -ItemType Directory -Path "data"
    }
    if (!(Test-Path "data\output")) {
        New-Item -ItemType Directory -Path "data\output"
    }

    Write-Host "Data directories created" -ForegroundColor Green

    # Run if requested
    if ($run) {
        Write-Host ""
        Write-Host "=== Running Arbitrage Analyzer ===" -ForegroundColor Blue
        Set-Location "build"
        & $executablePath
        Set-Location ".."
    }
}
catch {
    Write-Host "Error: $_" -ForegroundColor Red
    Set-Location ".."
    exit 1
}

Write-Host "Build script completed successfully!" -ForegroundColor Green
