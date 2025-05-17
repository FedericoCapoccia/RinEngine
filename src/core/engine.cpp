#include "engine.hpp"

#include "core/clock.hpp"
#include "core/logger.hpp"
#include "systems/renderer/renderer.hpp"
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

    clock::init();

    log::info("initializing engine");
    state = (state_t*)calloc(1, sizeof(state_t));
    state->app = app;

    if (!window::initialize(app->config.window_width, app->config.window_height, app->config.name)) {
        log::error("engine::create -> failed to initialize window system");
        shutdown();
        return false;
    }

    if (!renderer::initialize(app->config.name)) {
        log::error("engine::create -> failed to initialize rendering system");
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

    renderer::shutdown();
    window::shutdown();

    free(state);
    state = nullptr;
    log::info("engine shut down");
    clock::shutdown();
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
        clock::track_update();

        if (!renderer::draw()) {
            log::error("engine::run -> failed to draw frame");
        }

        clock::track_draw();
        clock::compute_frametime();

        window::poll();
        state->is_running = !window::should_close();
    }

    return true;
}

}
