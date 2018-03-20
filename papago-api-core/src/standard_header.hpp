#pragma once

#include <iostream>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define PAPAGO_ERROR(msg) throw new std::runtime_error(msg);