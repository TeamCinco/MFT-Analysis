# PowerShell setup script for Stock Visualizer dependencies

Write-Host "Setting up Stock Visualizer dependencies..." -ForegroundColor Yellow

# Create deps directory
if (!(Test-Path "deps")) {
    New-Item -ItemType Directory -Path "deps"
}

Set-Location "deps"

try {
    # Clone ImGui
    if (!(Test-Path "imgui")) {
        Write-Host "Cloning ImGui..." -ForegroundColor Yellow
        git clone https://github.com/ocornut/imgui.git
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ ImGui cloned" -ForegroundColor Green
        } else {
            throw "Failed to clone ImGui"
        }
    } else {
        Write-Host "✓ ImGui already exists" -ForegroundColor Green
    }

    # Clone ImPlot
    if (!(Test-Path "implot")) {
        Write-Host "Cloning ImPlot..." -ForegroundColor Yellow
        git clone https://github.com/epezent/implot.git
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ ImPlot cloned" -ForegroundColor Green
        } else {
            throw "Failed to clone ImPlot"
        }
    } else {
        Write-Host "✓ ImPlot already exists" -ForegroundColor Green
    }

    # Clone gl3w
    if (!(Test-Path "gl3w")) {
        Write-Host "Cloning gl3w..." -ForegroundColor Yellow
        git clone https://github.com/skaslev/gl3w.git
        if ($LASTEXITCODE -eq 0) {
            Set-Location "gl3w"
            
            # Try Python 3 first, then Python
            Write-Host "Generating gl3w headers..." -ForegroundColor Yellow
            python gl3w_gen.py
            if ($LASTEXITCODE -ne 0) {
                python3 gl3w_gen.py
                if ($LASTEXITCODE -ne 0) {
                    Write-Host "Warning: Could not generate gl3w headers. You may need to install Python." -ForegroundColor Yellow
                }
            }
            
            Set-Location ".."
            Write-Host "✓ gl3w cloned and generated" -ForegroundColor Green
        } else {
            throw "Failed to clone gl3w"
        }
    } else {
        Write-Host "✓ gl3w already exists" -ForegroundColor Green
    }

    Set-Location ".."
    Write-Host "All dependencies set up successfully!" -ForegroundColor Green
    Write-Host "You can now run .\build.ps1 to compile the project" -ForegroundColor Yellow
}
catch {
    Write-Host "Error: $_" -ForegroundColor Red
    Set-Location ".."
    exit 1
}
