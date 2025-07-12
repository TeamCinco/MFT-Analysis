cmake_minimum_required(VERSION 3.12)
project(StockVisualizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)

# Find GLFW
pkg_check_modules(GLFW3 REQUIRED glfw3)

# Include directories
include_directories(include)
include_directories(deps/imgui)
include_directories(deps/imgui/backends)
include_directories(deps/implot)
include_directories(deps/gl3w/include)

# Source files for original visualizer
set(SOURCES
    src/main.cpp
    src/StockVisualizer.cpp
    src/ChartRenderer.cpp
    src/FileManager.cpp
    src/UIComponents.cpp
)

# Source files for pairs visualizer
set(PAIRS_SOURCES
    src/pairs_main.cpp
    src/CointegrationVisualizer.cpp
)

# Source files for unified launcher
set(LAUNCHER_SOURCES
    src/launcher_main.cpp
    src/StockVisualizer.cpp
    src/ChartRenderer.cpp
    src/FileManager.cpp
    src/UIComponents.cpp
    src/CointegrationVisualizer.cpp
)

# ImGui sources
set(IMGUI_SOURCES
    deps/imgui/imgui.cpp
    deps/imgui/imgui_demo.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/backends/imgui_impl_glfw.cpp
    deps/imgui/backends/imgui_impl_opengl3.cpp
)

# ImPlot sources
set(IMPLOT_SOURCES
    deps/implot/implot.cpp
    deps/implot/implot_items.cpp
)

# gl3w source
set(GL3W_SOURCE
    deps/gl3w/src/gl3w.c
)

# Create original stock visualizer executable
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${IMGUI_SOURCES} 
    ${IMPLOT_SOURCES} 
    ${GL3W_SOURCE}
)

# Create pairs visualizer executable
add_executable(PairsVisualizer
    ${PAIRS_SOURCES} 
    ${IMGUI_SOURCES} 
    ${IMPLOT_SOURCES} 
    ${GL3W_SOURCE}
)

# Create unified launcher executable
add_executable(MFTAnalysis
    ${LAUNCHER_SOURCES} 
    ${IMGUI_SOURCES} 
    ${IMPLOT_SOURCES} 
    ${GL3W_SOURCE}
)

# Link libraries for original visualizer
target_link_libraries(${PROJECT_NAME} 
    ${GLFW3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Link libraries for pairs visualizer
target_link_libraries(PairsVisualizer 
    ${GLFW3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Link libraries for unified launcher
target_link_libraries(MFTAnalysis 
    ${GLFW3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

# Add GLFW library path for macOS
if(APPLE)
    target_link_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/lib)
    target_link_directories(PairsVisualizer PRIVATE /opt/homebrew/lib)
    target_link_directories(MFTAnalysis PRIVATE /opt/homebrew/lib)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${GLFW3_CFLAGS_OTHER})

target_include_directories(PairsVisualizer PRIVATE ${GLFW3_INCLUDE_DIRS})
target_compile_options(PairsVisualizer PRIVATE ${GLFW3_CFLAGS_OTHER})

target_include_directories(MFTAnalysis PRIVATE ${GLFW3_INCLUDE_DIRS})
target_compile_options(MFTAnalysis PRIVATE ${GLFW3_CFLAGS_OTHER})

# Compiler-specific options
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} pthread)
    target_link_libraries(PairsVisualizer pthread)
    target_link_libraries(MFTAnalysis pthread)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    target_link_libraries(PairsVisualizer "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    target_link_libraries(MFTAnalysis "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()
