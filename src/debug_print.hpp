#pragma once

#ifdef DEBUG_PRINT_ENABLED

#include <print>

#define DEBUG_PRINT(...) std::println(__VA_ARGS__)

#else

#define DEBUG_PRINT(...) do {} while (false)

#endif