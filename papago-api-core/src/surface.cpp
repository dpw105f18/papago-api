#include "standard_header.hpp"
#include "surface.hpp"

Surface::operator vk::SurfaceKHR&()
{
	return *m_vkSurfaceKHR;
}

uint32_t Surface::getWidth() const {
	return m_width;
}

uint32_t Surface::getHeight() const {
	return m_height;
}

Surface::Surface(uint32_t width, uint32_t height, HWND hwnd) : m_width(width), m_height(height)
{
	vk::ApplicationInfo appInfo("PapaGo-api", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	std::vector<const char*> surfaceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	m_vkInstance = createInstanceUnique(vk::InstanceCreateInfo()
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(surfaceExtensions.size())
		.setPpEnabledExtensionNames(surfaceExtensions.data()));

	m_vkSurfaceKHR = m_vkInstance->createWin32SurfaceKHRUnique(vk::Win32SurfaceCreateInfoKHR()
		.setHwnd(hwnd)
		.setHinstance(GetModuleHandle(nullptr)));
}
