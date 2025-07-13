# PowerShell build script for Windows
param(
    [switch]$run
)

Write-Host "Starting build process..." -ForegroundColor Blue

# Configuration
$EXECUTABLE_NAME = "run_feature_extractor"

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

# Create build directory if it doesn't exist
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build"
}

Set-Location "build"

try {
    # Run CMake to configure
    Write-Host "Configuring with CMake..." -ForegroundColor Yellow
    & $CMAKE_EXE ..
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build the project
    Write-Host "Compiling..." -ForegroundColor Yellow
    & $CMAKE_EXE --build . --config Release
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host "Executable is located at: .\build\$EXECUTABLE_NAME.exe" -ForegroundColor Green

    # Run if requested
    if ($run) {
        Write-Host ""
        Write-Host "--- Running Executable ---" -ForegroundColor Blue
        & ".\$EXECUTABLE_NAME.exe"
        Write-Host "--- Execution Finished ---" -ForegroundColor Blue
    }
}
catch {
    Write-Host "Error: $_" -ForegroundColor Red
    Set-Location ".."
    exit 1
}

Set-Location ".."
