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
