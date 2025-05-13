#pragma once

#include "core/application.hpp"

namespace rin::engine {

bool initialize(application* app);
void shutdown(void);
bool run(void);

}
