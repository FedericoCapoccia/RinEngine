#include "device.hpp"

#include "core/logger.hpp"
#include "loader.hpp"
#include "utils.hpp"

#include <vulkan/vk_enum_string_helper.h>

namespace rin::renderer::vulkan::device {

static device_t* device = nullptr;

static bool select_physical_device(const context_t*);
static bool select_queues(VkSurfaceKHR);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool create(context_t* context)
{
    if (device != nullptr) {
        log::error("vulkan::device::create -> a device instance already exists");
        return false;
    }

    device = (device_t*)calloc(1, sizeof(device_t));

    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 0,
        .pQueueCreateInfos = nullptr,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures = nullptr,
    };

    if (!select_physical_device(context)) {
        log::error("vulkan::device::create -> failed to select physical device");
        destroy();
        return false;
    }

    vkGetPhysicalDeviceProperties(device->physical_device, &device->properties);
    log::info("Selected: %s", device->properties.deviceName);
    vkGetPhysicalDeviceFeatures(device->physical_device, &device->features);
    vkGetPhysicalDeviceMemoryProperties(device->physical_device, &device->memory);

    darray<const char*> required_extensions { true };
    required_extensions.push("VK_KHR_swapchain");

    if (!utils::load_device_extensions(device->physical_device, &device_info, required_extensions)) {
        log::error("vulkan_device_create -> device does not supports all required extensions");
        destroy();
        return false;
    }

    if (!select_queues(context->surface)) {
        log::error("vulkan_device_create -> failed to scan for queue families");
        destroy();
        return false;
    }

    // NOTE: attaching Queue create infos
    darray<VkDeviceQueueCreateInfo> queue_infos { true };
    float priorities = 1.0f;

    VkDeviceQueueCreateInfo graphics_queue = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = (u32)device->graphics_queue.family,
        .queueCount = 1,
        .pQueuePriorities = &priorities,
    };
    queue_infos.push(graphics_queue);

    VkDeviceQueueCreateInfo compute_queue = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = (u32)device->compute_queue.family,
        .queueCount = 1,
        .pQueuePriorities = &priorities,
    };
    queue_infos.push(compute_queue);

    if (device->transfer_queue.dedicated) {
        VkDeviceQueueCreateInfo transfer_queue = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = (u32)device->transfer_queue.family,
            .queueCount = 1,
            .pQueuePriorities = &priorities,
        };
        queue_infos.push(transfer_queue);
    }

    device_info.pQueueCreateInfos = queue_infos.data;
    device_info.queueCreateInfoCount = queue_infos.len;

    VkPhysicalDeviceTimelineSemaphoreFeatures tim_sem = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
        .pNext = nullptr,
        .timelineSemaphore = VK_TRUE,
    };

    VkPhysicalDeviceSynchronization2Features sync2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .pNext = &tim_sem,
        .synchronization2 = VK_TRUE,
    };

    VkPhysicalDeviceDynamicRenderingFeatures dyn_ren = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .pNext = &sync2,
        .dynamicRendering = VK_TRUE,
    };

    device_info.pNext = &dyn_ren;

    // NOTE: create device
    VkResult result = vkCreateDevice(device->physical_device, &device_info, nullptr, &device->logical_device);
    if (result != VK_SUCCESS) {
        log::error("vulkan_device_create -> failed to create logical device: %s", string_VkResult(result));
        destroy();
        return false;
    }

    load_device(device->logical_device);

    vkGetDeviceQueue(device->logical_device, device->graphics_queue.family, 0, &device->graphics_queue.handle);
    vkGetDeviceQueue(device->logical_device, device->compute_queue.family, 0, &device->compute_queue.handle);
    vkGetDeviceQueue(device->logical_device, device->transfer_queue.family, 0, &device->transfer_queue.handle);

    context->device = device;
    device->context = context;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void destroy(void)
{
    if (device == nullptr) {
        return;
    }

    if (device->logical_device != VK_NULL_HANDLE) {
        vkDestroyDevice(device->logical_device, nullptr);
    }

    device->context->device = nullptr;
    free(device);
    device = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool select_physical_device(const context_t* context)
{
    u32 count = 0;
    vkEnumeratePhysicalDevices(context->instance, &count, nullptr);

    if (count == 0) {
        log::error("no physical device detected");
        return false;
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * count);
    vkEnumeratePhysicalDevices(context->instance, &count, devices);

    u32 max_score = 0;
    for (u32 i = 0; i < count; i++) {
        VkPhysicalDevice candidate = devices[i];
        VkPhysicalDeviceProperties props {};
        vkGetPhysicalDeviceProperties(candidate, &props);

        u32 score = 0;
        switch (props.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score = 10;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score = 5;
            break;
        default:
            score = 1;
        }

        if (score > max_score) {
            max_score = score;
            device->physical_device = candidate;
        }
    }

    free(devices);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool select_queues(VkSurfaceKHR surface)
{
    log::debug("Scanning for physical device queue support");
    u32 count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &count, nullptr);
    VkQueueFamilyProperties* props = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &count, props);

    for (u32 i = 0; i < count; i++) {
        VkQueueFlags flags = props[i].queueFlags;
        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device->physical_device, i, surface, &present);
        if (flags & VK_QUEUE_GRAPHICS_BIT && present == VK_TRUE) {
            log::debug("\tFound graphics queue family = %d", i);
            device->graphics_queue.family = i;
            device->graphics_queue.dedicated = true;
            break;
        }
    }

    if (device->graphics_queue.family == -1) {
        log::error("\tno queue family capable of graphics found");
        free(props);
        return false;
    }

    for (u32 i = 0; i < count; i++) {
        VkQueueFlags flags = props[i].queueFlags;
        if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT)) {
            log::debug("\tFound async compute queue family = %d", i);
            device->compute_queue.family = i;
            device->compute_queue.dedicated = true;
            break;
        }
    }

    if (device->compute_queue.family == -1) {
        log::error("\tno async compute queue family found");
        free(props);
        return false;
    }

    for (u32 i = 0; i < count; i++) {
        VkQueueFlags flags = props[i].queueFlags;
        if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT)) {
            log::debug("\tFound dedicated Transfer queue family = %d", i);
            device->transfer_queue.family = i;
            device->transfer_queue.dedicated = true;
            break;
        }
    }

    if (device->transfer_queue.family == -1) {
        log::warn("\tno dedicated transfer queue found, falling back to graphics queue");
        device->transfer_queue.family = device->graphics_queue.family;
        device->transfer_queue.dedicated = false;
    }

    free(props);
    return true;
}

}
