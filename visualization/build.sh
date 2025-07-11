#!/bin/bash

# build.sh - Build script for Stock Visualizer
# Usage: ./build.sh [clean|rebuild|run]

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project directories
PROJECT_DIR=$(pwd)
BUILD_DIR="$PROJECT_DIR/build"
DEPS_DIR="$PROJECT_DIR/deps"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install dependencies on different systems
install_dependencies() {
    print_status "Installing system dependencies..."
    
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Ubuntu/Debian
        if command_exists apt-get; then
            sudo apt-get update
            sudo apt-get install -y build-essential cmake libglfw3-dev libgl1-mesa-dev pkg-config
        # CentOS/RHEL/Fedora
        elif command_exists yum; then
            sudo yum groupinstall -y "Development Tools"
            sudo yum install -y cmake glfw-devel mesa-libGL-devel
        elif command_exists dnf; then
            sudo dnf groupinstall -y "Development Tools"
            sudo dnf install -y cmake glfw-devel mesa-libGL-devel
        else
            print_error "Unsupported Linux distribution"
            exit 1
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        if command_exists brew; then
            brew install cmake glfw
        else
            print_error "Homebrew not found. Please install Homebrew first:"
            print_error "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        fi
    else
        print_error "Unsupported operating system: $OSTYPE"
        exit 1
    fi
}

# Function to download and setup dependencies
setup_dependencies() {
    print_status "Setting up project dependencies..."
    
    mkdir -p "$DEPS_DIR"
    cd "$DEPS_DIR"
    
    # Download Dear ImGui
    if [ ! -d "imgui" ]; then
        print_status "Downloading Dear ImGui..."
        git clone https://github.com/ocornut/imgui.git
    else
        print_status "Dear ImGui already exists, updating..."
        cd imgui && git pull && cd ..
    fi
    
    # Download ImPlot
    if [ ! -d "implot" ]; then
        print_status "Downloading ImPlot..."
        git clone https://github.com/epezent/implot.git
    else
        print_status "ImPlot already exists, updating..."
        cd implot && git pull && cd ..
    fi
    
    # Download and setup gl3w
    if [ ! -d "gl3w" ]; then
        print_status "Downloading gl3w..."
        git clone https://github.com/skaslev/gl3w.git
        cd gl3w
        
        # Generate gl3w files
        if command_exists python3; then
            python3 gl3w_gen.py
        elif command_exists python; then
            python gl3w_gen.py
        else
            print_error "Python not found. Please install Python to generate gl3w files."
            exit 1
        fi
        cd ..
    else
        print_status "gl3w already exists"
    fi
    
    cd "$PROJECT_DIR"
}

# Function to create CMakeLists.txt if it doesn't exist
create_cmake_file() {
    print_status "Creating CMakeLists.txt..."
    cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(StockVisualizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/deps/imgui
    ${CMAKE_SOURCE_DIR}/deps/imgui/backends
    ${CMAKE_SOURCE_DIR}/deps/implot
    ${CMAKE_SOURCE_DIR}/deps/gl3w/include
)

# Source files
set(IMGUI_SOURCES
    deps/imgui/imgui.cpp
    deps/imgui/imgui_demo.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/backends/imgui_impl_glfw.cpp
    deps/imgui/backends/imgui_impl_opengl3.cpp
)

set(IMPLOT_SOURCES
    deps/implot/implot.cpp
    deps/implot/implot_items.cpp
)

set(GL3W_SOURCES
    deps/gl3w/src/gl3w.c
)

# Check if all source files exist
foreach(source_file ${IMGUI_SOURCES} ${IMPLOT_SOURCES} ${GL3W_SOURCES})
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${source_file}")
        message(FATAL_ERROR "Source file not found: ${source_file}")
    endif()
endforeach()

# Create executable
add_executable(StockVisualizer
    main.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
    ${GL3W_SOURCES}
)

# Link libraries
target_link_libraries(StockVisualizer
    OpenGL::GL
    glfw
    ${CMAKE_DL_LIBS}
)

# Set output directory
set_target_properties(StockVisualizer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy data directory to build directory if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/data")
    file(COPY "${CMAKE_SOURCE_DIR}/data" DESTINATION "${CMAKE_BINARY_DIR}")
endif()
EOF
    print_success "CMakeLists.txt created"
}

# Function to create a symlink to your data
setup_data_link() {
    local data_source="/Users/jazzhashzzz/Desktop/MFT-Analysis/results/7.11.25"
    local data_target="$PROJECT_DIR/data"
    
    if [ -d "$data_source" ]; then
        if [ ! -L "$data_target" ] && [ ! -d "$data_target" ]; then
            print_status "Creating symlink to data directory..."
            ln -s "$data_source" "$data_target"
            print_success "Data symlink created: $data_target -> $data_source"
        else
            print_status "Data directory already exists"
        fi
    else
        print_warning "Data source directory not found: $data_source"
        print_status "You can create a 'data' directory and place your CSV files there"
    fi
}

# Function to build the project
build_project() {
    print_status "Building project..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure with CMake
    print_status "Configuring with CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    # Build
    print_status "Compiling..."
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    cd "$PROJECT_DIR"
    print_success "Build completed successfully!"
}

# Function to run the application
run_application() {
    if [ -f "$BUILD_DIR/StockVisualizer" ]; then
        print_status "Running Stock Visualizer..."
        cd "$BUILD_DIR"
        ./StockVisualizer
    else
        print_error "Executable not found. Build first."
        exit 1
    fi
}

# Function to clean build files
clean_build() {
    print_status "Cleaning build files..."
    rm -rf "$BUILD_DIR"
    print_success "Build files cleaned"
}

# Function to show help
show_help() {
    echo "Stock Visualizer Build Script"
    echo ""
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  (no args)    Full setup and build"
    echo "  clean        Clean build files"
    echo "  rebuild      Clean and rebuild"
    echo "  run          Run the application"
    echo "  deps         Install system dependencies"
    echo "  help         Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0           # Full setup and build"
    echo "  $0 clean     # Clean build files"
    echo "  $0 rebuild   # Clean and rebuild"
    echo "  $0 run       # Run the application"
}

# Main script logic
main() {
    print_status "Stock Visualizer Build Script"
    print_status "=============================="
    
    case "${1:-}" in
        "clean")
            clean_build
            ;;
        "rebuild")
            clean_build
            setup_dependencies
            setup_data_link
            create_cmake_file
            build_project
            ;;
        "run")
            run_application
            ;;
        "deps")
            install_dependencies
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        "")
            # Full setup and build
            if ! command_exists cmake; then
                print_error "CMake not found. Installing dependencies..."
                install_dependencies
            fi
            
            setup_dependencies
            setup_data_link
            create_cmake_file
            build_project
            
            print_success "Build completed!"
            print_status "To run the application: $0 run"
            print_status "To clean build files: $0 clean"
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
}

# Check if main.cpp exists
if [ ! -f "main.cpp" ]; then
    print_error "main.cpp not found in current directory"
    print_error "Please run this script from the project root directory"
    exit 1
fi

# Run main function
main "$@"