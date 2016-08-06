#pragma once
#include "Core/Assertion.h"
#define NK_ASSERT(expr) o_assert(expr)
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

// FIXME: this is currently only needed for function with varargs
#define NK_INCLUDE_STANDARD_IO
