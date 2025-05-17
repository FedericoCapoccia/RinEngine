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
    "src/core/clock.cpp"
    "src/core/engine.cpp"
    "src/systems/window/window.cpp"

    "src/systems/renderer/renderer.cpp"
    "src/systems/renderer/vk/vma_impl.cpp"
    "src/systems/renderer/vk/loader.cpp"
    "src/systems/renderer/vk/context.cpp"
    "src/systems/renderer/vk/device.cpp"
    "src/systems/renderer/vk/swapchain.cpp"
    "src/systems/renderer/vk/pipeline.cpp"
    "src/systems/renderer/vk/utils.cpp"

    "src/systems/renderer/gui.cpp"
    "vendor/imgui/imgui.cpp"
    "vendor/imgui/imgui_demo.cpp"
    "vendor/imgui/imgui_draw.cpp"
    "vendor/imgui/imgui_tables.cpp"
    "vendor/imgui/imgui_widgets.cpp"
    "vendor/imgui/backends/imgui_impl_vulkan.cpp"
    "vendor/imgui/backends/imgui_impl_glfw.cpp"
)

set_source_files_properties(
    "src/systems/renderer/vk/vma_impl.cpp"
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
    "vendor/imgui"
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE 
    "GLFW_INCLUDE_NONE"
    "VK_NO_PROTOTYPES"
    "IMGUI_IMPL_VULKAN_USE_VOLK"
    $<$<CONFIG:Release>:RRELEASE>
)
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -fno-exceptions -fno-rtti -std=c++23)

if (WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE gdi32 winmm dwmapi)
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE)
else()
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE)
endif()

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.vert"
        "${PROJECT_SOURCE_DIR}/resources/shaders/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "BUILDING SHADER")
    #message(STATUS "${CMAKE_PROJECT_SOURCE_DIR}/resources/shaders/${FILE_NAME}.spv")
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/resources/shaders/${FILE_NAME}.spv")
    message(STATUS ${GLSL})
    ##execute glslang command to compile that specific shader
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${CMAKE_PROJECT_NAME} Shaders)
