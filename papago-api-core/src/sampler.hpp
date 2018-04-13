#pragma once
#include "device.hpp"
#include "api_enums.hpp"

class Sampler
{
public:

	Sampler(SamplerD dimension);

	explicit operator vk::Sampler&();

	Sampler& setMagFilter(Filter filter);
	Sampler& setMinFilter(Filter filter);
	Sampler& setTextureWrapU(TextureWrapMode mode);
	Sampler& setTextureWrapV(TextureWrapMode mode);
	Sampler& setTextureWrapW(TextureWrapMode mode);

private:
	friend class Device;
	vk::SamplerCreateInfo m_vkSamplerCreateInfo;
	vk::UniqueSampler m_vkTextureSampler;
	SamplerD m_dimension;

};