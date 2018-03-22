#pragma once
#include "device.hpp"

class Surface
{
public:
	Surface(uint32_t width, uint32_t height, HWND hwnd);
	
	uint32_t getWidth() const;
	uint32_t getHeight() const;

	explicit operator vk::SurfaceKHR&();
private:
	uint32_t m_width, m_height;
	vk::UniqueInstance m_vkInstance;
	vk::UniqueSurfaceKHR m_vkSurfaceKHR;
#ifdef PAPAGO_USE_VALIDATION_LAYERS
	vk::UniqueDebugReportCallbackEXT m_DebugReportCallback;
#endif

	static void checkInstanceLayers(std::vector<const char*> requiredLayers);

	friend std::vector<Device> Device::enumerateDevices(Surface&, const vk::PhysicalDeviceFeatures &, const std::vector<const char*> &);
};