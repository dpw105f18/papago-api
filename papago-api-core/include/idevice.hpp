#pragma once
#include "common.hpp"

class ISurface;
enum class Format;
class ISwapchain;
class IBufferResource;
class IImageResource;
class ISampler;
enum class Filter;
enum class TextureWrapMode;

class IDevice {
public:
	enum class PresentMode {
		eMailbox
	};

	virtual std::unique_ptr<ISwapchain> createSwapChain(Format, size_t framebufferCount, PresentMode) = 0;
	template<class T>
	std::unique_ptr<IBufferResource> createVertexBuffer(std::vector<T> data);
	template<class T>
	std::unique_ptr<IBufferResource> createIndexBuffer(std::vector<T> data);
	virtual std::unique_ptr<IBufferResource> createUniformBuffer(size_t size) = 0;
	virtual std::unique_ptr<ISampler> createTextureSampler1D(
		Filter magFilter, 
		Filter minFilter, 
		TextureWrapMode modeU) = 0;
	virtual std::unique_ptr<ISampler> createTextureSampler2D(
		Filter magFilter,
		Filter minFilter,
		TextureWrapMode modeU,
		TextureWrapMode modeV) = 0;
	virtual std::unique_ptr<ISampler> createTextureSampler3D(
		Filter magFilter,
		Filter minFilter,
		TextureWrapMode modeU,
		TextureWrapMode modeV,
		TextureWrapMode modeW) = 0;
	virtual std::unique_ptr<IImageResource> createTexture2D(size_t width, size_t height, Format) = 0;

	struct Features {
		bool samplerAnisotropy;
	};

	struct Extensions {
		bool swapchain;
		bool samplerMirrorClampToEdge;
	};

	PAPAGO_API static std::vector<std::unique_ptr<IDevice>> enumerateDevices(ISurface&, const Features&, const Extensions&);

protected:
	virtual std::unique_ptr<IBufferResource> createVertexBufferInternal(std::vector<char>& data) = 0;
	virtual std::unique_ptr<IBufferResource> createIndexBufferInternal(std::vector<char>& data) = 0;
};

template<class T>
inline std::unique_ptr<IBufferResource> IDevice::createVertexBuffer(std::vector<T> vertex_data) {
	size_t size = sizeof(T) * vertex_data.size();
	std::vector<char> data(size);
	memcpy(data.data(), vertex_data.data(), size);
	return createVertexBufferInternal(data);
}

template<>
inline std::unique_ptr<IBufferResource> IDevice::createIndexBuffer<uint32_t>(std::vector<uint32_t> index_data) {
	size_t size = sizeof(uint32_t) * index_data.size();
	std::vector<char> data(size);
	memcpy(data.data(), index_data.data(), size);
	return createIndexBufferInternal(data);
}

template<> 
inline std::unique_ptr<IBufferResource> IDevice::createIndexBuffer<uint16_t>(std::vector<uint16_t> index_data) {
	size_t size = sizeof(uint16_t) * index_data.size();
	std::vector<char> data(size);
	memcpy(data.data(), index_data.data(), size);
	return createIndexBufferInternal(data);
}

template<class T> inline std::unique_ptr<IBufferResource> IDevice::createIndexBuffer(std::vector<T>) {
	throw std::runtime_error("Only the types uint16 and uint32 can be used in index buffers.");
}