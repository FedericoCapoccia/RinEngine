#pragma once

#include "defines.hpp"

namespace rin::clock {

void init(void);
void reset(void);
void shutdown(void);
f64 get_time_s(void);
f64 track_update(void);
f64 track_draw(void);
void compute_frametime(void);
f64 get_frametime_ns(void);
f64 get_frametime_ms(void);
u64 get_fps(void);

}
