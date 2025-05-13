#include "engine.hpp"

#include "core/logger.hpp"
#include "systems/window/window.hpp"

#include <cstdlib>

namespace rin::engine {

struct state_t {
    application* app;
    bool is_running;
};

static state_t* state = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool initialize(application* app)
{
    if (state != nullptr) {
        log::error("engine::initialize -> engine already initialized");
        return false;
    }

    log::info("initializing engine");
    state = (state_t*)calloc(1, sizeof(state_t));
    state->app = app;

    if (!window::initialize(app->config.window_width, app->config.window_height, app->config.name)) {
        log::error("engine::create -> failed to initialize window system");
        shutdown();
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void shutdown(void)
{
    if (state == nullptr)
        return;

    window::shutdown();

    free(state);
    state = nullptr;
    log::info("engine shut down");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool run(void)
{
    if (state == nullptr) {
        log::error("engine::run -> engine not initalized");
        return false;
    }

    window::show();
    state->is_running = true;

    while (state->is_running) {
        window::poll();
        state->is_running = !window::should_close();
    }

    return true;
}

}
