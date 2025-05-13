#pragma once

#include "core/containers/darray.hpp"

#include <volk.h>

namespace rin::renderer::vulkan::utils {

bool load_instance_layers(VkInstanceCreateInfo* create_info, darray<const char*>& required_layers);
bool load_instance_extensions(VkInstanceCreateInfo* create_info, darray<const char*>& required_extensions);
bool load_device_extensions(VkPhysicalDevice device, VkDeviceCreateInfo* create_info, darray<const char*>& required_extensions);

}
