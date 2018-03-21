#pragma once
#include "device.hpp"

class Surface
{
public:
	Surface(uint32_t width, uint32_t height, HWND hwnd);
	
	uint32_t width() const;
	uint32_t height() const;

	explicit operator vk::SurfaceKHR&();
private:
	uint32_t m_width, m_height;
	vk::UniqueInstance m_vkInstance;
	vk::UniqueSurfaceKHR m_vkSurfaceKHR;

	friend std::vector<Device> Device::enumerateDevices(Surface&, const vk::PhysicalDeviceFeatures &, const std::vector<const char*> &);
};