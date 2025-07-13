# PowerShell build script for Stock Visualizer
param(
    [switch]$help
)

if ($help) {
    Write-Host "Usage: .\build.ps1"
    Write-Host "Note: Run .\setup.ps1 first to install dependencies"
    exit 0
}

Write-Host "Stock Visualizer Build Script" -ForegroundColor Yellow

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

# Check dependencies
Write-Host "Checking dependencies..." -ForegroundColor Yellow

$dependencies = @("imgui", "implot", "gl3w")
foreach ($dep in $dependencies) {
    if (Test-Path "deps\$dep") {
        Write-Host "✓ Found: $dep" -ForegroundColor Green
    } else {
        Write-Host "Missing dependency: $dep" -ForegroundColor Red
        Write-Host "Please run .\setup.ps1 first" -ForegroundColor Red
        exit 1
    }
}

# Create build directory
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build"
}

Set-Location "build"

try {
    # Configure with CMake
    Write-Host "Configuring with CMake..." -ForegroundColor Yellow
    & $CMAKE_EXE ..
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build
    Write-Host "Building..." -ForegroundColor Yellow
    & $CMAKE_EXE --build . --config Release --parallel $([Environment]::ProcessorCount)
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host "Build completed successfully!" -ForegroundColor Green
    
    # Check for executable
    if (Test-Path "Release\StockVisualizer.exe") {
        Write-Host "Executable: .\build\Release\StockVisualizer.exe" -ForegroundColor Green
    } elseif (Test-Path "StockVisualizer.exe") {
        Write-Host "Executable: .\build\StockVisualizer.exe" -ForegroundColor Green
    } else {
        Write-Host "Warning: Executable not found in expected location" -ForegroundColor Yellow
    }
}
catch {
    Write-Host "Error: $_" -ForegroundColor Red
    Set-Location ".."
    exit 1
}

Set-Location ".."

# Check data directory
if (!(Test-Path "data")) {
    Write-Host "Note: No 'data' directory found. The visualizer will read from the path specified in the data file." -ForegroundColor Yellow
}
