#include "standard_header.hpp"
#include "sampler.hpp"

Sampler& Sampler::setMagFilter(vk::Filter filter)
{
	m_vkSamplerCreateInfo.magFilter = filter;
	return *this;
}

Sampler& Sampler::setMinFilter(vk::Filter filter)
{
	m_vkSamplerCreateInfo.magFilter = filter;
	return *this;
}

Sampler& Sampler::setTextureWrapU(vk::SamplerAddressMode mode)
{
	m_vkSamplerCreateInfo.addressModeU = mode;
	return *this;
}

Sampler& Sampler::setTextureWrapV(vk::SamplerAddressMode mode)
{
	if (m_dimension < SamplerD::e2D)
	{
		throw std::runtime_error("Tried to set texturewrap on the 2nd dimension in a 1 dimensional sampler.");
	}

	m_vkSamplerCreateInfo.addressModeV = mode;
	return *this;
}

Sampler& Sampler::setTextureWrapW(vk::SamplerAddressMode mode)
{
	if (m_dimension < SamplerD::e3D) 
	{
		throw std::runtime_error("Tried to set texturewrap on the 3rd dimension in a 1 or 2 dimensional sampler.");
	}

	m_vkSamplerCreateInfo.addressModeW = mode;
	return *this;
}
