#include "swapchain.hpp"

#include "core/logger.hpp"

#include <algorithm>
#include <vulkan/vk_enum_string_helper.h>

namespace rin::renderer::vulkan::swapchain {

static swapchain_t* swapchain = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkPresentModeKHR choose_present_mode(context_t* context)
{
    u32 count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->device->physical_device, context->surface, &count, nullptr);
    VkPresentModeKHR* modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->device->physical_device, context->surface, &count, modes);

    for (u32 i = 0; i < count; i++) {
        if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            free(modes);
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    free(modes);
    return VK_PRESENT_MODE_FIFO_KHR;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkSurfaceFormatKHR choose_format(context_t* context)
{
    u32 count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->device->physical_device, context->surface, &count, nullptr);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->device->physical_device, context->surface, &count, formats);

    for (u32 i = 0; i < count; i++) {
        VkSurfaceFormatKHR format = formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            free(formats);
            return format;
        }
    }

    VkSurfaceFormatKHR format = formats[0];
    free(formats);
    return format;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VkExtent2D choose_extent(VkExtent2D current)
{
    VkSurfaceCapabilitiesKHR caps = swapchain->capabilities;
    if (caps.currentExtent.width != UINT32_MAX) {
        return caps.currentExtent;
    }

    VkExtent2D extent = current;
    extent.width = std::min(caps.maxImageExtent.width, std::max(caps.minImageExtent.width, extent.width));
    extent.height = std::min(caps.maxImageExtent.height, std::max(caps.minImageExtent.height, extent.height));
    return extent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool create(context_t* context, VkExtent2D window_extent)
{
    if (swapchain == nullptr) {
        if (context->swapchain != nullptr) {
            log::error("vulkan_context contains a swapchain already wtf???");
            return false;
        }

        log::debug("allocating vulkan_swapchain");
        swapchain = (swapchain_t*)calloc(1, sizeof(swapchain_t));
        swapchain->images = darray<VkImage>(true);
        swapchain->views = darray<VkImageView>(true);
        swapchain->render_semaphores = darray<VkSemaphore>(true);
        swapchain->context = context;
    }

    VkPhysicalDevice physical_device = swapchain->context->device->physical_device;
    VkDevice device = swapchain->context->device->logical_device;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, context->surface, &swapchain->capabilities);

    swapchain->present_mode = choose_present_mode(context);
    swapchain->format = choose_format(context);
    swapchain->extent = choose_extent(window_extent);

    // NOTE: MIN(x, y) and MAX(x, y) macros would make it more readable
    swapchain->min_image_count = swapchain->capabilities.minImageCount + 1;
    if (swapchain->capabilities.maxImageCount > 0) {
        if (swapchain->capabilities.maxImageCount < swapchain->min_image_count) {
            swapchain->min_image_count = swapchain->capabilities.maxImageCount;
        }
    }

    VkImageUsageFlags usage = 0;
    usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = context->surface,
        .minImageCount = swapchain->min_image_count,
        .imageFormat = swapchain->format.format,
        .imageColorSpace = swapchain->format.colorSpace,
        .imageExtent = swapchain->extent,
        .imageArrayLayers = 1,
        .imageUsage = usage,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = swapchain->capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapchain->present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    VkSwapchainKHR old_handle = swapchain->handle;
    if (old_handle != VK_NULL_HANDLE) {
        create_info.oldSwapchain = old_handle;
    }

    VkResult result = vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain->handle);
    if (result != VK_SUCCESS) {
        log::error("vulkan::swapchain::create -> failed to create swapchain: %s", string_VkResult(result));
        destroy();
        return false;
    }

    if (old_handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, old_handle, nullptr);
    }

    // NOTE: images
    u32 img_count = 0;
    vkGetSwapchainImagesKHR(device, swapchain->handle, &img_count, nullptr);
    swapchain->images.reserve(img_count);
    swapchain->images.len = img_count;
    vkGetSwapchainImagesKHR(device, swapchain->handle, &img_count, swapchain->images.data);
    swapchain->images.trim();

    // NOTE: image views
    swapchain->views.reserve(img_count);
    for (size_t i = 0; i < swapchain->images.len; i++) {
        VkImage img = swapchain->images[i];

        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = img,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->format.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        VkImageView view = VK_NULL_HANDLE;
        result = vkCreateImageView(device, &view_info, nullptr, &view);
        if (result != VK_SUCCESS) {
            log::error("vulkan::swapchain::create -> failed to create image view: %s", string_VkResult(result));
            destroy();
            return false;
        }
        swapchain->views.push(view);
    }
    swapchain->views.trim();

    // NOTE: semaphores
    swapchain->render_semaphores.reserve(img_count);
    for (size_t i = 0; i < swapchain->images.len; i++) {
        VkSemaphoreCreateInfo sem_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };

        VkSemaphore sem;
        result = vkCreateSemaphore(device, &sem_info, nullptr, &sem);
        if (result != VK_SUCCESS) {
            log::error("vulkan::swapchain::create -> failed to create semaphore: %s", string_VkResult(result));
            destroy();
            return false;
        }
        swapchain->render_semaphores.push(sem);
    }
    swapchain->render_semaphores.trim();

    context->swapchain = swapchain;
    swapchain->context = context;
    return true;
}

bool resize(VkExtent2D window_extent)
{
    if (swapchain == nullptr) {
        log::error("vulkan::swapchain::resize -> invalid swapchain");
        return false;
    }

    VkDevice device = swapchain->context->device->logical_device;
    vkDeviceWaitIdle(device);

    for (u32 i = 0; i < swapchain->images.len; i++) { // both views and render_semaphores have the same len as images
        vkDestroyImageView(device, swapchain->views[i], nullptr);
        vkDestroySemaphore(device, swapchain->render_semaphores[i], nullptr);
    }

    swapchain->images.clear();
    swapchain->views.clear();
    swapchain->render_semaphores.clear();

    if (!create(swapchain->context, window_extent)) {
        log::error("vulkan::swapchain::resize -> failed to recreate swapchain");
        return false;
    }

    return true;
}

void destroy(void)
{
    if (swapchain == nullptr) {
        return;
    }

    VkDevice device = swapchain->context->device->logical_device;
    vkDeviceWaitIdle(device);

    for (u32 i = 0; i < swapchain->images.len; i++) {
        vkDestroyImageView(device, swapchain->views[i], nullptr);
        vkDestroySemaphore(device, swapchain->render_semaphores[i], nullptr);
    }

    if (swapchain->handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, swapchain->handle, nullptr);
    }

    swapchain->context->swapchain = nullptr;
    free(swapchain);
    swapchain = nullptr;
}

}
