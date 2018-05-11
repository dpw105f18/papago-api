#pragma once
#include "isurface.hpp"

class Surface : public ISurface
{
public:
	Surface(uint32_t width, uint32_t height, HWND hwnd);

	explicit operator vk::SurfaceKHR&();
	vk::UniqueInstance m_vkInstance;
private:
	vk::UniqueSurfaceKHR m_vkSurfaceKHR;
#ifdef PAPAGO_USE_VALIDATION_LAYERS
	vk::UniqueDebugReportCallbackEXT m_debugReportCallback;
#endif

	static void checkInstanceLayers(const std::vector<const char*>& requiredLayers);
};