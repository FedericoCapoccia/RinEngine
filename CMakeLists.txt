cmake_minimum_required(VERSION 3.28)

project(RinEngine LANGUAGES CXX)

set(CMAKE_CXX_STARNDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_BUILD_INSTALL OFF CACHE BOOL "GLFW lib only")
add_subdirectory("vendor/glfw")

add_executable(${CMAKE_PROJECT_NAME})

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    "src/main.cpp"
    "src/core/logger.cpp"
    "src/core/engine.cpp"
    "src/systems/window/window.cpp"

    "src/systems/renderer/renderer.cpp"
    "src/systems/renderer/vulkan/vma_impl.cpp"
    "src/systems/renderer/vulkan/loader.cpp"
    "src/systems/renderer/vulkan/device.cpp"
    "src/systems/renderer/vulkan/swapchain.cpp"
    "src/systems/renderer/vulkan/utils.cpp"
    # "src/renderer/vulkan/context.c"
)

set_source_files_properties(
    "src/systems/renderer/vulkan/vma_impl.cpp"
    PROPERTIES
    COMPILE_FLAGS "-w"
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE 
    "src"
    "vendor/glfw/include"
    "vendor/vulkan/include"
    "vendor/vulkan_utils/include"
    "vendor/vma/include"
    "vendor/volk"
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
    "GLFW_INCLUDE_NONE"
    "VK_NO_PROTOTYPES"
    $<$<CONFIG:Release>:RRELEASE>
)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -fno-exceptions -fno-rtti -std=c++23)

if (WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE gdi32 winmm dwmapi)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE)
else()
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE)
endif()
