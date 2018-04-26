#pragma once

#include <sstream>

// include before Vulkan to make sure that max and min macros aren't defined
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <iostream>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define PAPAGO_ERROR(msg) throw std::runtime_error(msg);
#define ITERATE(collection) std::begin(collection), std::end(collection)
#define PAPAGO_EXPORT

#include "api_enums.hpp"

/* --------------------- Custom to vulkan translations --------------------- */

inline vk::Filter to_vulkan_filter(Filter filter) {
	switch (filter)
	{
	case Filter::eNearest:
		return vk::Filter::eNearest;
	case Filter::eLinear:
		return vk::Filter::eLinear;
	default:
		throw std::runtime_error("Unknown filter");
	}
}

inline vk::SamplerAddressMode to_vulkan_address_mode(TextureWrapMode mode) {
	switch (mode)
	{
	case TextureWrapMode::eClampToBorder:
		return vk::SamplerAddressMode::eClampToBorder;
	case TextureWrapMode::eClampToEdge:
		return vk::SamplerAddressMode::eClampToEdge;
	case TextureWrapMode::eMirroredRepeat:
		return vk::SamplerAddressMode::eMirroredRepeat;
	case TextureWrapMode::eMirrorClampToEdge:
		return vk::SamplerAddressMode::eMirrorClampToEdge;
	case TextureWrapMode::eRepeat:
		return vk::SamplerAddressMode::eRepeat;
	default:
		throw std::runtime_error("Unknown texture wrap mode");
	}
}

inline vk::Format to_vulkan_format(Format format) {
	switch (format)
	{
	case Format::eR8G8B8Unorm:
		return vk::Format::eR8G8B8Unorm;
	case Format::eR8G8B8A8Unorm:
		return vk::Format::eR8G8B8A8Unorm;
	case Format::eB8G8R8A8Unorm:
		return vk::Format::eB8G8R8A8Unorm;
	default:
		PAPAGO_ERROR("Invalid format");
		break;
	}
}

inline Format from_vulkan_format(vk::Format format) {
	switch (format) {
	case vk::Format::eR8G8B8Unorm:
		return Format::eR8G8B8Unorm;
	case vk::Format::eR8G8B8A8Unorm:
		return Format::eR8G8B8A8Unorm;
	case vk::Format::eB8G8R8A8Unorm:
			return Format::eB8G8R8A8Unorm;
	default:
		PAPAGO_ERROR("Invalid format");
		break;
	}
}
