#include "window.hpp"

#include "core/logger.hpp"
#include "systems/renderer/renderer.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#endif

namespace rin::window {

static GLFWwindow* window = nullptr;

static void on_error(int code, const char* message)
{
    log::error("GLFW error[%d]: %s", code, message);
}

static void on_resize(GLFWwindow* handle, i32 width, i32 height)
{
    (void)handle;
    (void)width;
    (void)height;
    rin::renderer::request_resize();
}

bool initialize(u32 width, u32 height, const char* title)
{
    if (window != nullptr) {
        log::error("window::initialize -> window system already initialized");
        return false;
    }

    glfwSetErrorCallback(on_error);

    if (!glfwInit()) {
        log::error("window::initialize -> failed to init GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (window == nullptr) {
        log::error("window::initialize -> failed to create primary window");
        shutdown();
        return false;
    }

    glfwSetFramebufferSizeCallback(window, on_resize);
    glfwSetWindowSizeLimits(window, 100, 100, GLFW_DONT_CARE, GLFW_DONT_CARE);

#ifdef _WIN32
    BOOL dark_mode = TRUE;
    HWND hwnd = glfwGetWin32Window(window);
    DwmSetWindowAttribute(hwnd, 20, &dark_mode, 4);
#endif

    return true;
}

void shutdown(void)
{
    if (window != nullptr) {
        glfwDestroyWindow(window);
    }

    glfwTerminate();
}
void show(void)
{
    glfwShowWindow(window);
}

void poll(void)
{
    glfwPollEvents();
}

bool should_close(void)
{
    return glfwWindowShouldClose(window);
}

void get_size(u32* out_width, u32* out_height)
{
    glfwGetFramebufferSize(window, (i32*)out_width, (i32*)out_height);
}

void get_monitor_size(u32* out_width, u32* out_height)
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    *out_width = mode->width;
    *out_height = mode->height;
}

void get_vulkan_extensions(darray<const char*>& buffer)
{
    u32 count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&count);
    for (u32 i = 0; i < count; i++) {
        buffer.push(glfw_extensions[i]);
    }
}

bool create_vulkan_surface(VkInstance instance, VkSurfaceKHR* out_surface)
{
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, out_surface);

    if (result != VK_SUCCESS) {
        log::error("window::create_vulkan_surface -> failed to create surface: %s", string_VkResult(result));
        return false;
    }

    return true;
}

}
