#pragma once

#include "types.hpp"

namespace rin::renderer::vulkan::context {

bool create(const char* app_name, bool enable_validation, context_t** out);
void destroy(void);

}
