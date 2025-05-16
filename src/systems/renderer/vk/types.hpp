#pragma once

#include "core/containers/darray.hpp"
#include "core/defines.hpp"

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
// clang-format on

namespace rin::renderer::vulkan {

struct context_t;

struct queue_t {
    VkQueue handle = VK_NULL_HANDLE;
    i32 family = -1;
    bool dedicated = false;
};

struct device_t {
    context_t* context;
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    queue_t graphics_queue;
    queue_t compute_queue;
    queue_t transfer_queue;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
};

struct swapchain_t {
    context_t* context;
    VkSwapchainKHR handle;
    VkExtent2D extent;
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    darray<VkImage> images;
    darray<VkImageView> views;
    u32 min_image_count;
    VkSurfaceCapabilitiesKHR capabilities;
    darray<VkSemaphore> render_semaphores;
    VkViewport viewport;
    VkRect2D scissor;
};

struct context_t {
    bool validation;
    VkInstance instance;
    VkDebugUtilsMessengerEXT messenger;
    VkSurfaceKHR surface;
    device_t* device;
    swapchain_t* swapchain;
    VmaAllocator vma;
};

enum image_type_t {
    IMAGE_TYPE_COLOR,
    IMAGE_TYPE_DEPTH,
};

struct image_create_info_t {
    VkFormat format;
    VkImageUsageFlags usage;
    u32 width, height;
    VmaAllocationCreateInfo allocation_info;
    image_type_t type;
};

struct image_t {
    VkImage handle;
    VkImageView view;
    u32 width, height;
    VkFormat format;
    VkImageUsageFlags usage;
    image_type_t type;
    VmaAllocation memory;
    VmaAllocationCreateInfo allocation_info;
};

}
