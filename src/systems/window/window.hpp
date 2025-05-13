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
void poll(void);

void get_vulkan_extensions(darray<const char*>& buffer);
bool create_vulkan_surface(VkInstance instance, VkSurfaceKHR* out_surface);

}
