#pragma once
//TODO: move diz!
using Format = vk::Format;
using SwapChainPresentMode = vk::PresentModeKHR;

enum class TypeEnums {
	eS32Float
};

enum class DepthTest {
	eLess
};

enum class ImageType {
	eImageDepthBuffer
};

enum class SamplerD {
	e1D = 1,
	e2D = 2,
	e3D = 3
};

enum Wrap {
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER
};

using Filter = vk::Filter;
/*
enum class Filter
{
	eNearest = VK_FILTER_NEAREST,
	eLinear = VK_FILTER_LINEAR,
	eCubicIMG = VK_FILTER_CUBIC_IMG
};
*/

enum class Usage {
	eReset,
	eReuse
};

using TextureWrapMode = vk::SamplerAddressMode;

using CommandBufferUsage = vk::CommandPoolCreateFlagBits;
