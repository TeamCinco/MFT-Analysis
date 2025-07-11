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
include_directories(src)
include_directories(deps/imgui)
include_directories(deps/imgui/backends)
include_directories(deps/implot)
include_directories(deps/gl3w/include)

# Source files
set(SOURCES
    src/main.cpp
    src/StockVisualizer.cpp
    src/ChartRenderer.cpp
    src/FileManager.cpp
    src/UIComponents.cpp
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

# Create executable
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${IMGUI_SOURCES} 
    ${IMPLOT_SOURCES} 
    ${GL3W_SOURCE}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${GLFW3_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW3_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PRIVATE ${GLFW3_CFLAGS_OTHER})

# Compiler-specific options
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()