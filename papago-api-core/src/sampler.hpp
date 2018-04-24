#pragma once
#include "device.hpp"
#include "api_enums.hpp"
#include "isampler.hpp"

class Sampler : public ISampler
{
public:

	Sampler(SamplerD dimension);

	explicit operator vk::Sampler&();

	Sampler& setMagFilter(vk::Filter filter);
	Sampler& setMinFilter(vk::Filter filter);
	Sampler& setTextureWrapU(vk::SamplerAddressMode mode);
	Sampler& setTextureWrapV(vk::SamplerAddressMode mode);
	Sampler& setTextureWrapW(vk::SamplerAddressMode mode);

private:
	friend class Device;
	vk::SamplerCreateInfo m_vkSamplerCreateInfo;
	vk::UniqueSampler m_vkTextureSampler;
	SamplerD m_dimension;

};