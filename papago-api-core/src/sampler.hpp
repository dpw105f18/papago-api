#pragma once
#include "device.hpp"
#include "api_enums.hpp"

//TODO: Check if constructors work at runtime.

class Sampler
{
public:

	Sampler(SamplerD dimension)
	{
		m_dimension = dimension;

		m_vkSamplerCreateInfo.magFilter = vk::Filter::eLinear;
		m_vkSamplerCreateInfo.minFilter = vk::Filter::eLinear;
		m_vkSamplerCreateInfo.anisotropyEnable = VK_TRUE;
		m_vkSamplerCreateInfo.maxAnisotropy = 16;
		m_vkSamplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		m_vkSamplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	}

	Sampler& setMagFilter(Filter filter);
	Sampler& setMinFilter(Filter filter);
	Sampler& setTextureWrapU(TextureWrapMode mode);
	Sampler& setTextureWrapV(TextureWrapMode mode);
	Sampler& setTextureWrapW(TextureWrapMode mode);

private:
	friend class Device;
	vk::SamplerCreateInfo m_vkSamplerCreateInfo;
	vk::UniqueSampler vk_mTextureSampler;
	SamplerD m_dimension;

};