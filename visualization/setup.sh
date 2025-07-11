#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}Setting up Stock Visualizer dependencies...${NC}"

# Create deps directory
mkdir -p deps
cd deps

# Clone ImGui
if [ ! -d "imgui" ]; then
    echo -e "${YELLOW}Cloning ImGui...${NC}"
    git clone https://github.com/ocornut/imgui.git
    echo -e "${GREEN}✓ ImGui cloned${NC}"
else
    echo -e "${GREEN}✓ ImGui already exists${NC}"
fi

# Clone ImPlot
if [ ! -d "implot" ]; then
    echo -e "${YELLOW}Cloning ImPlot...${NC}"
    git clone https://github.com/epezent/implot.git
    echo -e "${GREEN}✓ ImPlot cloned${NC}"
else
    echo -e "${GREEN}✓ ImPlot already exists${NC}"
fi

# Clone gl3w
if [ ! -d "gl3w" ]; then
    echo -e "${YELLOW}Cloning gl3w...${NC}"
    git clone https://github.com/skaslev/gl3w.git
    cd gl3w
    python3 gl3w_gen.py || python gl3w_gen.py
    cd ..
    echo -e "${GREEN}✓ gl3w cloned and generated${NC}"
else
    echo -e "${GREEN}✓ gl3w already exists${NC}"
fi

cd ..

echo -e "${GREEN}All dependencies set up successfully!${NC}"
echo -e "${YELLOW}You can now run ./build.sh to compile the project${NC}"