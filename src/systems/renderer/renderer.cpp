#include "renderer.hpp"

namespace rin::renderer {

struct state_t {};

struct state_t* state = nullptr;

bool initialize(const char* app_name);

void shutdown(void);

}
