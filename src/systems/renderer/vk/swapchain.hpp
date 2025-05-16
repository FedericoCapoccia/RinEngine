#pragma once

#include "types.hpp"

namespace rin::renderer::vulkan::swapchain {

bool create(context_t* context, VkExtent2D window_extent);
bool resize(VkExtent2D window_extent);
void destroy(void);

}
