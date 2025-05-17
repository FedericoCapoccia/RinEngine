#include "clock.hpp"

#include "core/containers/darray.hpp"

#include <chrono>
#include <cmath>

namespace rin::clock {

static constexpr u32 FPS_CAPTURE_FRAMES_COUNT = 30;
static constexpr f64 FPS_AVERAGE_TIME_SECONDS = 0.5f;
static constexpr f64 FPS_STEP = FPS_AVERAGE_TIME_SECONDS / FPS_CAPTURE_FRAMES_COUNT;

using Clock = std::chrono::high_resolution_clock;

struct clock_t {
    Clock timer;
    Clock::time_point start_time;
    f64 current_time;
    f64 previous_time;
    f64 update_dt;
    f64 draw_dt;
    f64 target;
    f64 frametime;
    u64 frame_counter;

    u32 fps_index;
    darray<f64> fps_history;
    f64 fps_average;
    f64 fps_last;
};

static clock_t* clock = nullptr;

void init(void)
{
    if (clock != nullptr) {
        return;
    }

    clock = (clock_t*)calloc(1, sizeof(clock_t));
    clock->timer = Clock {};
    clock->fps_history = darray<f64> { FPS_CAPTURE_FRAMES_COUNT, true };
    clock->start_time = Clock::now();
}

void reset(void)
{
    clock->start_time = Clock::now();
    clock->frame_counter = 0;
}

void shutdown(void)
{
    free(clock);
    clock = nullptr;
}

f64 get_time_s(void)
{
    auto now = Clock::now();
    return std::chrono::duration<f64>(now - clock->start_time).count();
}

f64 track_update(void)
{
    clock->current_time = get_time_s();
    clock->update_dt = clock->current_time - clock->previous_time;
    clock->previous_time = clock->current_time;
    return clock->update_dt;
}

f64 track_draw(void)
{
    clock->current_time = get_time_s();
    clock->draw_dt = clock->current_time - clock->previous_time;
    clock->previous_time = clock->current_time;
    return clock->draw_dt;
}

void compute_frametime(void)
{
    clock->frametime = clock->update_dt + clock->draw_dt;
    clock->frame_counter += 1;
}

f64 get_frametime_ns(void)
{
    return clock->frametime * ns_per_s;
}

f64 get_frametime_ms(void)
{
    return clock->frametime * ms_per_s;
}

u64 get_fps(void)
{
    if (clock->frame_counter == 0) {
        clock->fps_average = 0;
        clock->fps_last = 0;
        clock->fps_index = 0;
        for (u32 i = 0; i < clock->fps_history.capacity; i++) {
            clock->fps_history[i] = 0;
        }
    }

    if (clock->frametime == 0) {
        return 0;
    }

    if ((get_time_s() - clock->fps_last) > FPS_STEP) {
        clock->fps_last = get_time_s();
        clock->fps_index = (clock->fps_index + 1) % FPS_CAPTURE_FRAMES_COUNT;
        clock->fps_average -= clock->fps_history[clock->fps_index];
        clock->fps_history[clock->fps_index] = clock->frametime / FPS_CAPTURE_FRAMES_COUNT;
        clock->fps_average += clock->fps_history[clock->fps_index];
    }

    return std::roundf(1.0 / clock->fps_average);
}

}
