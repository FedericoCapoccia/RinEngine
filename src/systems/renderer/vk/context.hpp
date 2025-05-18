#pragma once

#include "systems/renderer/vk/types.hpp"

#include <glm/glm.hpp>

namespace rin::renderer::vulkan::context {

bool create(const char* app_name, bool enable_validation, context_t** out);
void destroy(void);

void begin_label(VkCommandBuffer cmd, const char* name, const glm::vec4& color);
void end_label(VkCommandBuffer cmd);
bool allocate_image(const image_create_info_t& info, image_t* out);
bool allocate_buffer(const buffer_create_info_t& info, buffer_t* out);

inline VkImageMemoryBarrier2 image_layout_transition(
    VkImage image, VkImageAspectFlags aspect_mask,
    VkImageLayout src_layout, VkImageLayout dst_layout,
    VkAccessFlags2 src_access, VkAccessFlags2 dst_access,
    VkPipelineStageFlags2 src_stage, VkPipelineStageFlags2 dst_stage)
{
    return VkImageMemoryBarrier2 {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = src_stage,
        .srcAccessMask = src_access,
        .dstStageMask = dst_stage,
        .dstAccessMask = dst_access,
        .oldLayout = src_layout,
        .newLayout = dst_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect_mask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
}

}
