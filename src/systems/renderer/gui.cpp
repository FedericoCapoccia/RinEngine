#include "gui.hpp"

#include "core/logger.hpp"

#include <backends/imgui_impl_vulkan.h>
#include <cmath>
#include <imgui.h>

namespace rin::renderer::gui {

static gui_t* state;

static f32 linearize_color(f32 srgb)
{
    if (srgb <= 0.04045) {
        return srgb / 12.92;
    }

    return std::pow((srgb + 0.055) / 1.055, 2.4);
}

bool initialize(vulkan::context_t* vk_context, gui_t** out);

void shutdown(void);

void on_resize(u32 min_image_count)
{
    ImGui_ImplVulkan_SetMinImageCount(min_image_count);
}

void draw(VkCommandBuffer cmd)
{
    ImDrawData* data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(data, cmd);
}

}
