cmake_minimum_required(VERSION 3.20)

project(CPU_RayTracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third_party/glfw)

set(IMGUI_DIR third_party/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw opengl32)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")

add_executable(CPU_RayTracer ${SOURCES})

target_include_directories(CPU_RayTracer PRIVATE src)
target_include_directories(CPU_RayTracer PRIVATE src third_party/tinyobjloader)
target_include_directories(CPU_RayTracer PRIVATE src third_party/stb)
target_link_libraries(CPU_RayTracer PRIVATE imgui)

add_custom_command(TARGET CPU_RayTracer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory $<TARGET_FILE_DIR:CPU_RayTracer>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_CURRENT_SOURCE_DIR}/assets 
        $<TARGET_FILE_DIR:CPU_RayTracer>/assets
)
