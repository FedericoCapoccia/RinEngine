#include "core/application.hpp"
#include "core/engine.hpp"
#include "core/logger.hpp"

#include <cstdlib>

using namespace rin;

int main(void)
{
    application_config app_info {
        .name = "Test",
        .window_width = 1280,
        .window_height = 720,
    };

    application app {
        .config = app_info,
        .p_user_data = nullptr,
    };

    if (!engine::initialize(&app)) {
        log::error("failed to initialize engine");
        return EXIT_FAILURE;
    }

    if (!engine::run()) {
        log::error("engine run loop failed");
        engine::shutdown();
        return EXIT_FAILURE;
    }

    engine::shutdown();
    return EXIT_SUCCESS;
}
