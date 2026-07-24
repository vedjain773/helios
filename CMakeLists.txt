cmake_minimum_required(VERSION 3.16)

project(helios LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/include/imgui)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_library(imgui
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui
    PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui
    PUBLIC
        glfw
        OpenGL::GL
)

add_executable(helios
    src/glad.c
    src/main.cpp
    src/shader.cpp
    src/compute.cpp
    src/camera.cpp
    src/renderer.cpp
    src/scene.cpp
)

target_include_directories(helios
    PUBLIC
        include
        include/glm-master/
)

target_link_libraries(helios
    PRIVATE
        imgui
        glfw
        OpenGL::GL
)
