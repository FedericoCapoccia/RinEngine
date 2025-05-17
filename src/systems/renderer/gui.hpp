#pragma once

#include "vk/types.hpp"

#include <imgui.h>

namespace rin::renderer::gui {

bool initialize(vulkan::context_t* vk_context);
void shutdown(void);
void on_resize(u32 min_image_count);
void draw(VkCommandBuffer cmd);
void prepare(void);

}
