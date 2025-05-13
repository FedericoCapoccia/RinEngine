#pragma once

#include "core/defines.hpp"

namespace rin {

struct application_config {
    const char* name;
    u32 window_width;
    u32 window_height;
};

struct application {
    application_config config;
    void* p_user_data;
};

}
