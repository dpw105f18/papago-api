#include "standard_header.hpp"
#include "surface.hpp"

Surface::operator vk::SurfaceKHR&()
{
	return m_VkSurfaceKHR;
}

uint32_t Surface::getWidth()
{
	return width;
}

uint32_t Surface::getHeight()
{
	return height;
}
