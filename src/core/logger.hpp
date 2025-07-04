#pragma once

namespace rin::log {

void error(const char* format, ...);
void warn(const char* format, ...);
void debug(const char* format, ...);
void info(const char* format, ...);

}
