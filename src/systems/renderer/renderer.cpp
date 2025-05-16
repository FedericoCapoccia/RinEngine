#include "renderer.hpp"

#include "core/logger.hpp"
#include "systems/window/window.hpp"
#include "vk/context.hpp"
#include "vk/pipeline.hpp"
#include "vk/swapchain.hpp"
#include "vk/types.hpp"
#include "vk/utils.hpp"

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
    // vulkan::image_t render_target;
    VkSemaphore image_acquired;
    VkFence fence;
    VkCommandPool pool;
    VkCommandBuffer cmd;
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
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

    VkDevice device = state->context->device->logical_device;

    VkSemaphoreCreateInfo sem_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkResult result = vkCreateSemaphore(device, &sem_info, nullptr, &state->image_acquired);
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

    result = vkCreateFence(device, &fence_info, nullptr, &state->fence);
    if (result != VK_SUCCESS) {
        log::error("renderer::initialize -> failed to create fence: %s", string_VkResult(result));
        shutdown();
        return false;
    }

    // TODO: Render target != swapchain

    // u32 monitor_width, monitor_height;
    // window::get_monitor_size(&monitor_width, &monitor_height);

    // vulkan::image_create_info_t img_info {
    //     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
    //     .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    //     .width = monitor_width,
    //     .height = monitor_height,
    //     .allocation_info = {
    //         .flags = 0,
    //         .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    //         .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    //         .preferredFlags = 0,
    //         .memoryTypeBits = 0,
    //         .pool = VK_NULL_HANDLE,
    //         .pUserData = nullptr,
    //         .priority = 0,
    //     },
    //     .type = vulkan::IMAGE_TYPE_COLOR,
    // };

    // if (!vulkan::context::allocate_image(img_info, &state->render_target)) {
    //     log::error("renderer::initialize -> failed to allocate render target");
    //     shutdown();
    //     return false;
    // }

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
    result = vkAllocateCommandBuffers(device, &alloc_info, &state->cmd);
    if (result != VK_SUCCESS) {
        log::error("renderer::initialize -> failed to allocate command buffer: %s", string_VkResult(result));
        shutdown();
        return false;
    }

    // TODO: load and create VkShaderModule

    VkShaderModule vert_mod, frag_mod;

    if (!vulkan::utils::load_shader_module(device, "resources/shaders/triangle.vert.spv", &vert_mod)) {
        log::error("renderer::initialize -> failed to load vertex shader module");
        shutdown();
        return false;
    }

    if (!vulkan::utils::load_shader_module(device, "resources/shaders/triangle.frag.spv", &frag_mod)) {
        log::error("renderer::initialize -> failed to load fragment shader module");
        vkDestroyShaderModule(device, vert_mod, nullptr);
        shutdown();
        return false;
    }

    VkPipelineLayoutCreateInfo layout_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };
    result = vkCreatePipelineLayout(device, &layout_info, nullptr, &state->pipeline_layout);
    if (result != VK_SUCCESS) {
        log::error("renderer::initialize -> failed to create pipeline layout: %s", string_VkResult(result));
        shutdown();
        return false;
    }

    vulkan::pipeline_builder_t pipeline_builder {};
    pipeline_builder
        .set_multisampling_none()
        .disable_blending()
        .disable_depthtest()
        .set_color_attachment_format(state->context->swapchain->format.format)
        .set_depth_format(VK_FORMAT_UNDEFINED)
        .set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
        .set_polygon_mode(VK_POLYGON_MODE_FILL)
        .set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
        .set_shaders(vert_mod, frag_mod)
        .set_layout(state->pipeline_layout);

    if (!pipeline_builder.build(state->context->device->logical_device, &state->pipeline)) {
        log::error("renderer::initialize -> failed to create offscreen rendering pipeline");
        vkDestroyShaderModule(device, vert_mod, nullptr);
        vkDestroyShaderModule(device, frag_mod, nullptr);
        shutdown();
        return false;
    }

    vkDestroyShaderModule(device, vert_mod, nullptr);
    vkDestroyShaderModule(device, frag_mod, nullptr);

    return true;
}

void shutdown(void)
{
    if (state == nullptr) {
        return;
    }

    VkDevice device = state->context->device->logical_device;
    vkDeviceWaitIdle(device);

    if (state->pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, state->pipeline_layout, nullptr);
    }

    if (state->pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, state->pipeline, nullptr);
    }

    if (state->pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, state->pool, nullptr);
    }

    if (state->image_acquired != VK_NULL_HANDLE) {
        vkDestroySemaphore(device, state->image_acquired, nullptr);
    }

    if (state->fence != VK_NULL_HANDLE) {
        vkDestroyFence(device, state->fence, nullptr);
    }

    // if (state->render_target.handle != VK_NULL_HANDLE) {
    //     vkDestroyImageView(device, state->render_target.view, nullptr);
    //     vmaDestroyImage(state->context->vma, state->render_target.handle, state->render_target.memory);
    // }

    vulkan::context::destroy();
    state->context = nullptr;

    free(state);
    state = nullptr;
}

void request_resize(void)
{
    state->resize_requested = true;
}

static bool resize(void)
{
    u32 width = 0, height = 0;
    window::get_size(&width, &height);

    while (width == 0 || height == 0) {
        window::wait_events();
        window::get_size(&width, &height);
    }

    if (!vulkan::swapchain::resize({ width, height })) {
        return false;
    }

    state->resize_requested = false;
    return true;
}

bool draw(void)
{
    VkDevice device = state->context->device->logical_device;
    vulkan::swapchain_t* swapchain = state->context->swapchain;
    VkResult vk_result = VK_SUCCESS;
    bool suboptimal = false;

    vk_result = vkWaitForFences(device, 1, &state->fence, VK_TRUE, UINT64_MAX);
    if (vk_result != VK_SUCCESS) {
        log::error("renderer::draw -> failed to wait for fences: %s", string_VkResult(vk_result));
        return false;
    }

    u32 image_index;
    vk_result = vkAcquireNextImageKHR(device, swapchain->handle, UINT64_MAX, state->image_acquired, VK_NULL_HANDLE, &image_index);

    switch (vk_result) {
    case VK_ERROR_OUT_OF_DATE_KHR:
        if (!resize()) {
            log::error("renderer::draw -> failed to resize swapchain");
            return false;
        }
        return true;
        break;
    case VK_SUBOPTIMAL_KHR:
        suboptimal = true;
        break;
    case VK_SUCCESS:
        break;
    default:
        log::error("renderer::draw -> failed to acquire swapchain image index: %s", string_VkResult(vk_result));
        return false;
    };

    vk_result = vkResetFences(device, 1, &state->fence);
    if (vk_result != VK_SUCCESS) {
        log::error("renderer::draw -> failed to reset fence: %s", string_VkResult(vk_result));
        return false;
    }

    vk_result = vkResetCommandPool(device, state->pool, 0);
    if (vk_result != VK_SUCCESS) {
        log::error("renderer::draw -> failed to reset command pool: %s", string_VkResult(vk_result));
        return false;
    }

    VkCommandBufferBeginInfo cmd_begin {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };
    vkBeginCommandBuffer(state->cmd, &cmd_begin);
    if (vk_result != VK_SUCCESS) {
        log::error("renderer::draw -> failed to begin command buffer: %s", string_VkResult(vk_result));
        return false;
    }

    {
        vulkan::context::begin_label(state->cmd, "color attachment transition", { .data = { 1.f, 0.f, 0.f, 1.f } });
        VkImageMemoryBarrier2 before_rendering = vulkan::context::image_layout_transition(
            swapchain->images[image_index], VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_ACCESS_2_NONE,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

        VkDependencyInfo dep {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = nullptr,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &before_rendering,
        };
        vkCmdPipelineBarrier2(state->cmd, &dep);
        vulkan::context::end_label(state->cmd);
    }

    // NOTE: rendering
    VkRenderingAttachmentInfo color_attachment {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = swapchain->views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .resolveImageView = VK_NULL_HANDLE,
        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
            .color = { .float32 = { 0.0, 0.0, 0.0, 1.0 } },
        },
    };

    VkRenderingInfo rendering {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderArea = {
            .offset = { 0, 0 },
            .extent = swapchain->extent,
        },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr,
    };

    vkCmdBeginRendering(state->cmd, &rendering);
    vulkan::context::begin_label(state->cmd, "Rendering", { .data = { 1.f, 0.f, 0.f, 1.f } });
    vkCmdBindPipeline(state->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state->pipeline);
    vkCmdSetViewport(state->cmd, 0, 1, &swapchain->viewport);
    vkCmdSetScissor(state->cmd, 0, 1, &swapchain->scissor);
    vkCmdDraw(state->cmd, 3, 1, 0, 0);
    vulkan::context::end_label(state->cmd);
    vkCmdEndRendering(state->cmd);

    {
        vulkan::context::begin_label(state->cmd, "present mode transition", { .data = { 1.f, 0.f, 0.f, 1.f } });
        VkImageMemoryBarrier2 to_present = vulkan::context::image_layout_transition(
            swapchain->images[image_index], VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);

        VkDependencyInfo dep {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0,
            .memoryBarrierCount = 0,
            .pMemoryBarriers = nullptr,
            .bufferMemoryBarrierCount = 0,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &to_present,
        };
        vkCmdPipelineBarrier2(state->cmd, &dep);
        vulkan::context::end_label(state->cmd);
    }

    vkEndCommandBuffer(state->cmd);

    // NOTE: submit

    VkCommandBufferSubmitInfo cmd_submit {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = state->cmd,
        .deviceMask = 0,
    };

    VkSemaphoreSubmitInfo wait_submit {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = state->image_acquired,
        .value = 0,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .deviceIndex = 0,
    };

    VkSemaphoreSubmitInfo signal_submit {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = swapchain->render_semaphores[image_index],
        .value = 0,
        .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
        .deviceIndex = 0,
    };

    VkSubmitInfo2 submit_info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        .flags = 0,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &wait_submit,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmd_submit,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signal_submit,
    };

    vk_result = vkQueueSubmit2(state->context->device->graphics_queue.handle, 1, &submit_info, state->fence);
    if (vk_result != VK_SUCCESS) {
        log::error("renderer::draw -> failed to submit command buffer: %s", string_VkResult(vk_result));
        return false;
    }

    // NOTE: present

    VkPresentInfoKHR present {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &swapchain->render_semaphores[image_index],
        .swapchainCount = 1,
        .pSwapchains = &swapchain->handle,
        .pImageIndices = &image_index,
        .pResults = nullptr,
    };

    vk_result = vkQueuePresentKHR(state->context->device->graphics_queue.handle, &present);
    switch (vk_result) {
    case VK_ERROR_OUT_OF_DATE_KHR:
        if (!resize()) {
            log::error("renderer::draw -> failed to resize swapchain");
            return false;
        }
        return true;
        break;
    case VK_SUBOPTIMAL_KHR:
        suboptimal = true;
        break;
    case VK_SUCCESS:
        break;
    default:
        log::error("renderer::draw -> failed to acquire swapchain image index: %s", string_VkResult(vk_result));
        return false;
    };

    if (suboptimal) {
        if (!resize()) {
            log::error("renderer::draw -> failed to resize swapchain");
            return false;
        }
    }

    // TODO: increment frame

    return true;
}

}
