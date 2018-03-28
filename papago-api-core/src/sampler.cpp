#include "standard_header.hpp"
#include "sampler.hpp"

Sampler& Sampler::setMagFilter(Filter filter)
{
	m_vkSamplerCreateInfo.magFilter = filter;
	return *this;
}

Sampler& Sampler::setMinFilter(Filter filter)
{
	m_vkSamplerCreateInfo.magFilter = filter;
	return *this;
}

Sampler& Sampler::setTextureWrapU(TextureWrapMode mode)
{
	m_vkSamplerCreateInfo.addressModeU = mode;
	return *this;
}

Sampler& Sampler::setTextureWrapV(TextureWrapMode mode)
{
	if (m_dimension < SamplerD::e2D)
	{
		throw std::runtime_error("Tried to set texturewrap on the 2nd dimension in a 1 dimensional sampler.");
	}

	m_vkSamplerCreateInfo.addressModeV = mode;
	return *this;
}

Sampler& Sampler::setTextureWrapW(TextureWrapMode mode)
{
	if (m_dimension < SamplerD::e3D) 
	{
		throw std::runtime_error("Tried to set texturewrap on the 3rd dimension in a 1 or 2 dimensional sampler.");
	}

	m_vkSamplerCreateInfo.addressModeW = mode;

	return *this;
}
