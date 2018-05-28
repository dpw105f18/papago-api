#pragma once

#include <sstream>

// include before Vulkan to make sure that max and min macros aren't defined
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <iostream>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define PAPAGO_ERROR(msg) throw std::runtime_error(__FILE__ ":" + std::to_string(__LINE__) + ": " + msg);
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
	case Format::eS8Uint:
		return vk::Format::eS8Uint;
	case Format::eD32Sfloat:
		return vk::Format::eD32Sfloat;
	case Format::eD32SfloatS8Uint:
		return vk::Format::eD32SfloatS8Uint;
	case Format::eD24UnormS8Uint:
		return vk::Format::eD24UnormS8Uint;
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
	case vk::Format::eD32Sfloat:
		return Format::eD32Sfloat;
	case vk::Format::eD32SfloatS8Uint:
		return Format::eD32SfloatS8Uint;
	case vk::Format::eD24UnormS8Uint:
		return Format::eD24UnormS8Uint;
	case vk::Format::eS8Uint:
		return Format::eS8Uint;
	default:
		PAPAGO_ERROR("Invalid format");
		break;
	}
}

inline size_t sizeOfFormat(vk::Format format)
{
	size_t result = 0;
	switch (format)
	{
	case vk::Format::eUndefined:
		break;
	case vk::Format::eR4G4UnormPack8:
		break;
	case vk::Format::eR4G4B4A4UnormPack16:
		break;
	case vk::Format::eB4G4R4A4UnormPack16:
		break;
	case vk::Format::eR5G6B5UnormPack16:
		break;
	case vk::Format::eB5G6R5UnormPack16:
		break;
	case vk::Format::eR5G5B5A1UnormPack16:
		break;
	case vk::Format::eB5G5R5A1UnormPack16:
		break;
	case vk::Format::eA1R5G5B5UnormPack16:
		break;
	case vk::Format::eR8Unorm:
		break;
	case vk::Format::eR8Snorm:
		break;
	case vk::Format::eR8Uscaled:
		break;
	case vk::Format::eR8Sscaled:
		break;
	case vk::Format::eR8Uint:
		break;
	case vk::Format::eR8Sint:
		break;
	case vk::Format::eR8Srgb:
		break;
	case vk::Format::eR8G8Unorm:
		break;
	case vk::Format::eR8G8Snorm:
		break;
	case vk::Format::eR8G8Uscaled:
		break;
	case vk::Format::eR8G8Sscaled:
		break;
	case vk::Format::eR8G8Uint:
		break;
	case vk::Format::eR8G8Sint:
		break;
	case vk::Format::eR8G8Srgb:
		break;
	case vk::Format::eR8G8B8Unorm:
		break;
	case vk::Format::eR8G8B8Snorm:
		break;
	case vk::Format::eR8G8B8Uscaled:
		break;
	case vk::Format::eR8G8B8Sscaled:
		break;
	case vk::Format::eR8G8B8Uint:
		break;
	case vk::Format::eR8G8B8Sint:
		break;
	case vk::Format::eR8G8B8Srgb:
		break;
	case vk::Format::eB8G8R8Unorm:
		break;
	case vk::Format::eB8G8R8Snorm:
		break;
	case vk::Format::eB8G8R8Uscaled:
		break;
	case vk::Format::eB8G8R8Sscaled:
		break;
	case vk::Format::eB8G8R8Uint:
		break;
	case vk::Format::eB8G8R8Sint:
		break;
	case vk::Format::eB8G8R8Srgb:
		break;
	case vk::Format::eR8G8B8A8Unorm:
	case vk::Format::eR8G8B8A8Snorm:
	case vk::Format::eR8G8B8A8Uscaled:
	case vk::Format::eR8G8B8A8Sscaled:
	case vk::Format::eR8G8B8A8Uint:
	case vk::Format::eR8G8B8A8Sint:
	case vk::Format::eR8G8B8A8Srgb:
	case vk::Format::eB8G8R8A8Unorm:
	case vk::Format::eB8G8R8A8Snorm:
	case vk::Format::eB8G8R8A8Uscaled:
	case vk::Format::eB8G8R8A8Sscaled:
	case vk::Format::eB8G8R8A8Uint:
	case vk::Format::eB8G8R8A8Sint:
	case vk::Format::eB8G8R8A8Srgb:
		return 4;
		break;
	case vk::Format::eA8B8G8R8UnormPack32:
		break;
	case vk::Format::eA8B8G8R8SnormPack32:
		break;
	case vk::Format::eA8B8G8R8UscaledPack32:
		break;
	case vk::Format::eA8B8G8R8SscaledPack32:
		break;
	case vk::Format::eA8B8G8R8UintPack32:
		break;
	case vk::Format::eA8B8G8R8SintPack32:
		break;
	case vk::Format::eA8B8G8R8SrgbPack32:
		break;
	case vk::Format::eA2R10G10B10UnormPack32:
		break;
	case vk::Format::eA2R10G10B10SnormPack32:
		break;
	case vk::Format::eA2R10G10B10UscaledPack32:
		break;
	case vk::Format::eA2R10G10B10SscaledPack32:
		break;
	case vk::Format::eA2R10G10B10UintPack32:
		break;
	case vk::Format::eA2R10G10B10SintPack32:
		break;
	case vk::Format::eA2B10G10R10UnormPack32:
		break;
	case vk::Format::eA2B10G10R10SnormPack32:
		break;
	case vk::Format::eA2B10G10R10UscaledPack32:
		break;
	case vk::Format::eA2B10G10R10SscaledPack32:
		break;
	case vk::Format::eA2B10G10R10UintPack32:
		break;
	case vk::Format::eA2B10G10R10SintPack32:
		break;
	case vk::Format::eR16Unorm:
		break;
	case vk::Format::eR16Snorm:
		break;
	case vk::Format::eR16Uscaled:
		break;
	case vk::Format::eR16Sscaled:
		break;
	case vk::Format::eR16Uint:
		break;
	case vk::Format::eR16Sint:
		break;
	case vk::Format::eR16Sfloat:
		break;
	case vk::Format::eR16G16Unorm:
		break;
	case vk::Format::eR16G16Snorm:
		break;
	case vk::Format::eR16G16Uscaled:
		break;
	case vk::Format::eR16G16Sscaled:
		break;
	case vk::Format::eR16G16Uint:
		break;
	case vk::Format::eR16G16Sint:
		break;
	case vk::Format::eR16G16Sfloat:
		break;
	case vk::Format::eR16G16B16Unorm:
		break;
	case vk::Format::eR16G16B16Snorm:
		break;
	case vk::Format::eR16G16B16Uscaled:
		break;
	case vk::Format::eR16G16B16Sscaled:
		break;
	case vk::Format::eR16G16B16Uint:
		break;
	case vk::Format::eR16G16B16Sint:
		break;
	case vk::Format::eR16G16B16Sfloat:
		break;
	case vk::Format::eR16G16B16A16Unorm:
		break;
	case vk::Format::eR16G16B16A16Snorm:
		break;
	case vk::Format::eR16G16B16A16Uscaled:
		break;
	case vk::Format::eR16G16B16A16Sscaled:
		break;
	case vk::Format::eR16G16B16A16Uint:
		break;
	case vk::Format::eR16G16B16A16Sint:
		break;
	case vk::Format::eR16G16B16A16Sfloat:
		break;
	case vk::Format::eR32Uint:
		break;
	case vk::Format::eR32Sint:
		break;
	case vk::Format::eR32Sfloat:
		break;
	case vk::Format::eR32G32Uint:
		break;
	case vk::Format::eR32G32Sint:
		break;
	case vk::Format::eR32G32Sfloat:
		break;
	case vk::Format::eR32G32B32Uint:
		break;
	case vk::Format::eR32G32B32Sint:
		break;
	case vk::Format::eR32G32B32Sfloat:
		break;
	case vk::Format::eR32G32B32A32Uint:
		break;
	case vk::Format::eR32G32B32A32Sint:
		break;
	case vk::Format::eR32G32B32A32Sfloat:
		break;
	case vk::Format::eR64Uint:
		break;
	case vk::Format::eR64Sint:
		break;
	case vk::Format::eR64Sfloat:
		break;
	case vk::Format::eR64G64Uint:
		break;
	case vk::Format::eR64G64Sint:
		break;
	case vk::Format::eR64G64Sfloat:
		break;
	case vk::Format::eR64G64B64Uint:
		break;
	case vk::Format::eR64G64B64Sint:
		break;
	case vk::Format::eR64G64B64Sfloat:
		break;
	case vk::Format::eR64G64B64A64Uint:
		break;
	case vk::Format::eR64G64B64A64Sint:
		break;
	case vk::Format::eR64G64B64A64Sfloat:
		break;
	case vk::Format::eB10G11R11UfloatPack32:
		break;
	case vk::Format::eE5B9G9R9UfloatPack32:
		break;
	case vk::Format::eD16Unorm:
		break;
	case vk::Format::eX8D24UnormPack32:
		break;
	case vk::Format::eD32Sfloat:
		return 4;
		break;
	case vk::Format::eS8Uint:
		return 1;
		break;
	case vk::Format::eD16UnormS8Uint:
		break;
	case vk::Format::eD24UnormS8Uint:
		return 4;
		break;
	case vk::Format::eD32SfloatS8Uint:
		return 5;
		break;
	case vk::Format::eBc1RgbUnormBlock:
		break;
	case vk::Format::eBc1RgbSrgbBlock:
		break;
	case vk::Format::eBc1RgbaUnormBlock:
		break;
	case vk::Format::eBc1RgbaSrgbBlock:
		break;
	case vk::Format::eBc2UnormBlock:
		break;
	case vk::Format::eBc2SrgbBlock:
		break;
	case vk::Format::eBc3UnormBlock:
		break;
	case vk::Format::eBc3SrgbBlock:
		break;
	case vk::Format::eBc4UnormBlock:
		break;
	case vk::Format::eBc4SnormBlock:
		break;
	case vk::Format::eBc5UnormBlock:
		break;
	case vk::Format::eBc5SnormBlock:
		break;
	case vk::Format::eBc6HUfloatBlock:
		break;
	case vk::Format::eBc6HSfloatBlock:
		break;
	case vk::Format::eBc7UnormBlock:
		break;
	case vk::Format::eBc7SrgbBlock:
		break;
	case vk::Format::eEtc2R8G8B8UnormBlock:
		break;
	case vk::Format::eEtc2R8G8B8SrgbBlock:
		break;
	case vk::Format::eEtc2R8G8B8A1UnormBlock:
		break;
	case vk::Format::eEtc2R8G8B8A1SrgbBlock:
		break;
	case vk::Format::eEtc2R8G8B8A8UnormBlock:
		break;
	case vk::Format::eEtc2R8G8B8A8SrgbBlock:
		break;
	case vk::Format::eEacR11UnormBlock:
		break;
	case vk::Format::eEacR11SnormBlock:
		break;
	case vk::Format::eEacR11G11UnormBlock:
		break;
	case vk::Format::eEacR11G11SnormBlock:
		break;
	case vk::Format::eAstc4x4UnormBlock:
		break;
	case vk::Format::eAstc4x4SrgbBlock:
		break;
	case vk::Format::eAstc5x4UnormBlock:
		break;
	case vk::Format::eAstc5x4SrgbBlock:
		break;
	case vk::Format::eAstc5x5UnormBlock:
		break;
	case vk::Format::eAstc5x5SrgbBlock:
		break;
	case vk::Format::eAstc6x5UnormBlock:
		break;
	case vk::Format::eAstc6x5SrgbBlock:
		break;
	case vk::Format::eAstc6x6UnormBlock:
		break;
	case vk::Format::eAstc6x6SrgbBlock:
		break;
	case vk::Format::eAstc8x5UnormBlock:
		break;
	case vk::Format::eAstc8x5SrgbBlock:
		break;
	case vk::Format::eAstc8x6UnormBlock:
		break;
	case vk::Format::eAstc8x6SrgbBlock:
		break;
	case vk::Format::eAstc8x8UnormBlock:
		break;
	case vk::Format::eAstc8x8SrgbBlock:
		break;
	case vk::Format::eAstc10x5UnormBlock:
		break;
	case vk::Format::eAstc10x5SrgbBlock:
		break;
	case vk::Format::eAstc10x6UnormBlock:
		break;
	case vk::Format::eAstc10x6SrgbBlock:
		break;
	case vk::Format::eAstc10x8UnormBlock:
		break;
	case vk::Format::eAstc10x8SrgbBlock:
		break;
	case vk::Format::eAstc10x10UnormBlock:
		break;
	case vk::Format::eAstc10x10SrgbBlock:
		break;
	case vk::Format::eAstc12x10UnormBlock:
		break;
	case vk::Format::eAstc12x10SrgbBlock:
		break;
	case vk::Format::eAstc12x12UnormBlock:
		break;
	case vk::Format::eAstc12x12SrgbBlock:
		break;
	case vk::Format::ePvrtc12BppUnormBlockIMG:
		break;
	case vk::Format::ePvrtc14BppUnormBlockIMG:
		break;
	case vk::Format::ePvrtc22BppUnormBlockIMG:
		break;
	case vk::Format::ePvrtc24BppUnormBlockIMG:
		break;
	case vk::Format::ePvrtc12BppSrgbBlockIMG:
		break;
	case vk::Format::ePvrtc14BppSrgbBlockIMG:
		break;
	case vk::Format::ePvrtc22BppSrgbBlockIMG:
		break;
	case vk::Format::ePvrtc24BppSrgbBlockIMG:
		break;
	case vk::Format::eG8B8G8R8422UnormKHR:
		break;
	case vk::Format::eB8G8R8G8422UnormKHR:
		break;
	case vk::Format::eG8B8R83Plane420UnormKHR:
		break;
	case vk::Format::eG8B8R82Plane420UnormKHR:
		break;
	case vk::Format::eG8B8R83Plane422UnormKHR:
		break;
	case vk::Format::eG8B8R82Plane422UnormKHR:
		break;
	case vk::Format::eG8B8R83Plane444UnormKHR:
		break;
	case vk::Format::eR10X6UnormPack16KHR:
		break;
	case vk::Format::eR10X6G10X6Unorm2Pack16KHR:
		break;
	case vk::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16KHR:
		break;
	case vk::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16KHR:
		break;
	case vk::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16KHR:
		break;
	case vk::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16KHR:
		break;
	case vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16KHR:
		break;
	case vk::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16KHR:
		break;
	case vk::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16KHR:
		break;
	case vk::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16KHR:
		break;
	case vk::Format::eR12X4UnormPack16KHR:
		break;
	case vk::Format::eR12X4G12X4Unorm2Pack16KHR:
		break;
	case vk::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16KHR:
		break;
	case vk::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16KHR:
		break;
	case vk::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16KHR:
		break;
	case vk::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16KHR:
		break;
	case vk::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16KHR:
		break;
	case vk::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16KHR:
		break;
	case vk::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16KHR:
		break;
	case vk::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16KHR:
		break;
	case vk::Format::eG16B16G16R16422UnormKHR:
		break;
	case vk::Format::eB16G16R16G16422UnormKHR:
		break;
	case vk::Format::eG16B16R163Plane420UnormKHR:
		break;
	case vk::Format::eG16B16R162Plane420UnormKHR:
		break;
	case vk::Format::eG16B16R163Plane422UnormKHR:
		break;
	case vk::Format::eG16B16R162Plane422UnormKHR:
		break;
	case vk::Format::eG16B16R163Plane444UnormKHR:
		break;
	default:
		PAPAGO_ERROR("How the hell did you end up here!?");
		break;
	}

	if (result == 0) {
		PAPAGO_ERROR("sizeOfFormat not implemented for this format!");
	}
	return result;
}

inline DepthStencilFlags GetDepthStencilFlags(vk::Format format)
{
	DepthStencilFlags flags = DepthStencilFlags::eNone;

	switch (format)
	{
	case vk::Format::eUndefined:
		break;
	
	case vk::Format::eD16Unorm:
	case vk::Format::eD32Sfloat:
		flags = DepthStencilFlags::eDepth;
		break;
	case vk::Format::eS8Uint:
		flags = DepthStencilFlags::eStencil;
		break;
	case vk::Format::eD16UnormS8Uint:
	case vk::Format::eD24UnormS8Uint:
	case vk::Format::eD32SfloatS8Uint:
		flags = DepthStencilFlags::eDepth | DepthStencilFlags::eStencil;
		break;
	}

	return flags;
}