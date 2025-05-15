#include "renderer.hpp"

#include "core/logger.hpp"
#include "systems/window/window.hpp"
#include "vk/context.hpp"
#include "vk/types.hpp"

#include <vulkan/vk_enum_string_helper.h>

namespace rin::renderer {

#ifdef RRELEASE
static const bool ENABLE_VALIDATION = false;
#else
static const bool ENABLE_VALIDATION = true;
#endif

struct state_t {
    vulkan::context_t* context;
    bool resize_requested;
    vulkan::image_t render_target;
    VkSemaphore image_acquired;
    VkFence fence;
    VkCommandPool pool;
    VkCommandBuffer cmd;
};

struct state_t* state = nullptr;

bool initialize(const char* app_name)
{
    if (state != nullptr) {
        log::error("renderer::initialize -> renderer system has been already initialized");
        return false;
    }

    state = (state_t*)calloc(1, sizeof(state_t));

    if (!vulkan::context::create(app_name, ENABLE_VALIDATION, &state->context)) {
        log::error("renderer::initialize -> failed to create vulkan context");
        shutdown();
        return false;
    }

    VkSemaphoreCreateInfo sem_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkResult result = vkCreateSemaphore(state->context->device->logical_device, &sem_info, nullptr, &state->image_acquired);
    if (result != VK_SUCCESS) {
        log::error("renderer::initialize -> failed to create semaphore: %s", string_VkResult(result));
        shutdown();
        return false;
    }

    VkFenceCreateInfo fence_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    result = vkCreateFence(state->context->device->logical_device, &fence_info, nullptr, &state->fence);
    if (result != VK_SUCCESS) {
        log::error("renderer::initialize -> failed to create fence: %s", string_VkResult(result));
        shutdown();
        return false;
    }

    u32 monitor_width, monitor_height;
    window::get_monitor_size(&monitor_width, &monitor_height);

    vulkan::image_create_info_t img_info {
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .width = monitor_width,
        .height = monitor_height,
        .allocation_info = {
            .flags = 0,
            .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool = VK_NULL_HANDLE,
            .pUserData = nullptr,
            .priority = 0,
        },
        .type = vulkan::IMAGE_TYPE_COLOR,
    };

    if (!vulkan::context::allocate_image(img_info, &state->render_target)) {
        log::error("renderer::initialize -> failed to allocate render target");
        shutdown();
        return false;
    }

    VkCommandPoolCreateInfo pool_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = (u32)state->context->device->graphics_queue.family,
    };
    result = vkCreateCommandPool(state->context->device->logical_device, &pool_info, nullptr, &state->pool);

    if (result != VK_SUCCESS) {
        log::error("renderer::initialize -> failed to create command pool: %s", string_VkResult(result));
        shutdown();
        return false;
    }

    VkCommandBufferAllocateInfo alloc_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = state->pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    result = vkAllocateCommandBuffers(state->context->device->logical_device, &alloc_info, &state->cmd);
    if (result != VK_SUCCESS) {
        log::error("renderer::initialize -> failed to allocate command buffer: %s", string_VkResult(result));
        shutdown();
        return false;
    }

    return true;
}

void shutdown(void)
{
    if (state == nullptr) {
        return;
    }

    VkDevice device = state->context->device->logical_device;

    if (state->pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, state->pool, nullptr);
    }

    if (state->image_acquired != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, state->image_acquired, nullptr);
    }

    if (state->fence != VK_NULL_HANDLE) {
        vkDestroyFence(device, state->fence, nullptr);
    }

    if (state->render_target.handle != VK_NULL_HANDLE) {
        vkDestroyImageView(device, state->render_target.view, nullptr);
        vmaDestroyImage(state->context->vma, state->render_target.handle, state->render_target.memory);
    }

    vulkan::context::destroy();
    state->context = nullptr;

    free(state);
    state = nullptr;
}

void request_resize(void)
{
    state->resize_requested = true;
}

bool draw(void)
{
    return true;
}

}
