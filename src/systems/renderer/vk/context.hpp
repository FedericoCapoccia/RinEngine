#pragma once

#include "core/math/types.hpp"
#include "systems/renderer/vk/types.hpp"

namespace rin::renderer::vulkan::context {

bool create(const char* app_name, bool enable_validation, context_t** out);
void destroy(void);

void begin_label(VkCommandBuffer cmd, const char* name, const vec4f_t& color);
void end_label(VkCommandBuffer cmd);
bool allocate_image(const image_create_info_t& info, image_t* out);

}
