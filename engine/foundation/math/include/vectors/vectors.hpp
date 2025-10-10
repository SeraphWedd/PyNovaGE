#pragma once

// Prevent Windows.h from defining min/max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Undefine Windows min/max macros if already defined
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
