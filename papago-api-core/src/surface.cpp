#include "standard_header.hpp"
#include "surface.hpp"

Surface::operator vk::SurfaceKHR&()
{
	return m_VkSurfaceKHR;
}
