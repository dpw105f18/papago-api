#pragma once

enum class Format {
	eR8G8B8Unorm,
	eR8G8B8A8Unorm,
	eB8G8R8A8Unorm
};

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

enum class Wrap {
	eClampToEdge,
	eClampToBorder
};

enum class Filter {
	eNearest,
	eLinear
};

enum class Usage {
	eReset,
	eReuse
};

enum class TextureWrapMode {
	eClampToBorder,
	eClampToEdge,
	eMirroredRepeat,
	eMirrorClampToEdge,
	eRepeat
};

enum class CommandBufferUsage {};
