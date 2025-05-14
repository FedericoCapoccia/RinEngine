#include "renderer.hpp"

#include "core/logger.hpp"
#include "vk/context.hpp"
#include "vk/types.hpp"

namespace rin::renderer {

#ifdef RRELEASE
static const bool ENABLE_VALIDATION = false;
#else
static const bool ENABLE_VALIDATION = true;
#endif

struct state_t {
    vulkan::context_t* context;
};

struct state_t* state = nullptr;

bool initialize(const char* app_name)
{
    if (state != nullptr) {
        log::error("renderer::initialize -> renderer system has been already initialized");
        return false;
    }

    state = (state_t*)calloc(1, sizeof(state_t));

    if (!vulkan::context::create(app_name, ENABLE_VALIDATION, &state->context)) {
        log::error("renderer::initialize -> failed to create vulkan context");
        shutdown();
        return false;
    }

    return true;
}

void shutdown(void)
{
    if (state == nullptr) {
        return;
    }

    vulkan::context::destroy();
    state->context = nullptr;

    free(state);
    state = nullptr;
}

}
