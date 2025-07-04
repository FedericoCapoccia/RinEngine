#pragma once

#include "core/containers/darray.hpp"
#include "core/defines.hpp"

#include <volk.h>

namespace rin::window {

bool initialize(u32 width, u32 height, const char* title);
void shutdown(void);
void show(void);
bool should_close(void);
void get_size(u32* out_width, u32* out_height);
void get_scale(f32* xscale, f32* yscale);
void get_monitor_size(u32* out_width, u32* out_height);
void poll(void);
void wait_events(void);

void get_vulkan_extensions(darray<const char*>& buffer);
bool create_vulkan_surface(VkInstance instance, VkSurfaceKHR* out_surface);
void init_imgui_vulkan(void);
void shutdown_imgui(void);

}
