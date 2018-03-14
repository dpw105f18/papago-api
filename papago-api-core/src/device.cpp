#include "standard_header.hpp"
#include "device.hpp"
#include <iostream>

std::vector<Device> Device::enumerateDevices()
{
	vk::ApplicationInfo appInfo("PapaGo-api", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	vk::InstanceCreateInfo info;
	info.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(extensions.size())
		.setPpEnabledExtensionNames(extensions.data());

	m_VkInstance = vk::createInstance(info);

	auto physicalDevices = m_VkInstance.enumeratePhysicalDevices();

	for (auto& device : physicalDevices) {
		std::cout << "Device found!" << std::endl;
	}

	return std::vector<Device>();
}

vk::Instance Device::m_VkInstance;