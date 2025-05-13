#pragma once

#include <volk.h>

namespace rin::renderer::vulkan {

bool load_core(void);
void load_instance(VkInstance instance);
void load_device(VkDevice device);

}
