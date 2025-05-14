#pragma once

#include "core/defines.hpp"

struct vec2f_t {
    union {
        f32 data[2];
        float x, y;
        float r, g;
    };
};

struct vec3f_t {
    union {
        f32 data[3];
        float x, y, z;
        float r, g, b;
    };
};

struct vec4f_t {
    union {
        f32 data[4];
        float x, y, z, w;
        float r, g, b, a;
    };
};
