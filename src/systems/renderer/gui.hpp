#pragma once

#include "vk/types.hpp"

#include <imgui.h>

namespace rin::renderer::gui {

struct gui_t {
    ImGuiContext* context;
    VkDescriptorPool pool;
    VkDevice device;
};

bool initialize(vulkan::context_t* vk_context, gui_t** out);
void shutdown(void);
void on_resize(u32 min_image_count);
void draw(VkCommandBuffer cmd);

}
