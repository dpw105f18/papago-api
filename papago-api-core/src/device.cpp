#include "standard_header.hpp"
#include "surface.hpp"
#include "device.hpp"
#include <iostream>
#include <set>

Surface Device::createSurface(HWND& hwnd)
{
	vk::ApplicationInfo appInfo("PapaGo-api", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	std::vector<const char*> surfaceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	vk::InstanceCreateInfo info;
	info.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(surfaceExtensions.size())
		.setPpEnabledExtensionNames(surfaceExtensions.data());

	m_VkInstance = vk::createInstance(info);

	vk::SurfaceKHR surface;

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.setHwnd(hwnd)
		.setHinstance(GetModuleHandle(nullptr));

	auto CreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(m_VkInstance.getProcAddr("vkCreateWin32SurfaceKHR"));

	if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(static_cast<VkInstance>(m_VkInstance), reinterpret_cast<VkWin32SurfaceCreateInfoKHR*>(&surfaceCreateInfo), nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)))
	{
		throw new std::runtime_error("Could not create Win32 surface");
	}

	return Surface(surface);
}

//Provides a vector of devices with the given [features] and [extensions] enabled
std::vector<Device> Device::enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions)
{
	//TODO: check to see that m_VkInstance is set


	//*************************************************************************

	std::vector<Device> result;

	auto physicalDevices = m_VkInstance.enumeratePhysicalDevices();

	for (auto& device : physicalDevices) {

		auto queueFamilies = device.getQueueFamilyProperties();

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		int graphicsQueueFamily = -1;
		int presentQueueFamily = -1;

		for (auto i = 0; i < queueFamilies.size(); ++i) {
			auto queueFamily = queueFamilies.at(i);

			if (queueFamily.queueCount > 0) {
				if ((graphicsQueueFamily == -1 || graphicsQueueFamily == presentQueueFamily) && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
				{
					graphicsQueueFamily = i;
				}

				if ((presentQueueFamily == -1 || presentQueueFamily == graphicsQueueFamily) && device.getSurfaceSupportKHR(i, surface)) {
					presentQueueFamily = i;
				}
			}
		}

		std::set<int> uniqueQueueFamilyIndicies;
		if (graphicsQueueFamily != -1) {
			uniqueQueueFamilyIndicies.emplace(graphicsQueueFamily);
		}

		if (presentQueueFamily != -1) {
			uniqueQueueFamilyIndicies.emplace(presentQueueFamily);
		}

		for (auto qf : uniqueQueueFamilyIndicies) {
			const float queuePriority = 1.0f;	//<-- TODO: should this be setable somehow?

			vk::DeviceQueueCreateInfo dqci = {};
			dqci.setQueueCount(1)					//<-- TODO: setable?
				.setPQueuePriorities(&queuePriority)
				.setQueueFamilyIndex(qf);

			queueCreateInfos.push_back(dqci);
		}
		

		vk::DeviceCreateInfo dci = {};
		dci.setEnabledExtensionCount(extensions.size())
			.setPpEnabledExtensionNames(extensions.data())
			.setPEnabledFeatures(&features)
			.setEnabledLayerCount(0)
			.setQueueCreateInfoCount(queueCreateInfos.size())
			.setPQueueCreateInfos(queueCreateInfos.data());

		auto logicalDevice = device.createDevice(dci);

		result.emplace_back(Device(device, logicalDevice));
	}

	return result;
}

vk::Instance Device::m_VkInstance;

