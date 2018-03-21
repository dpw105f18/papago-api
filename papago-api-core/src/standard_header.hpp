#pragma once

// include before Vulkan to make sure that max and min macros aren't defined
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <iostream>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define PAPAGO_ERROR(msg) throw new std::runtime_error(msg);