#include "standard_header.hpp"
#include "surface.hpp"
#include "device.hpp"
#include <iostream>
#include <set>
#include <algorithm>
#include "swap_chain.hpp"
#include "image_resource.hpp"
#include <memory>

Surface Device::createSurface(size_t width, size_t height, HWND& hwnd)
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

	s_VkInstance = vk::createInstance(info);

	vk::SurfaceKHR surface;

	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.setHwnd(hwnd)
		.setHinstance(GetModuleHandle(nullptr));

	auto CreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(s_VkInstance.getProcAddr("vkCreateWin32SurfaceKHR"));

	if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(static_cast<VkInstance>(s_VkInstance), reinterpret_cast<VkWin32SurfaceCreateInfoKHR*>(&surfaceCreateInfo), nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)))
	{
		throw new std::runtime_error("Could not create Win32 surface");
	}

	return Surface(width, height, surface);
}

//Provides a vector of devices with the given [features] and [extensions] enabled
std::vector<Device> Device::enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions)
{
	//TODO: check to see that m_VkInstance is set


	//*************************************************************************

	std::vector<Device> result;

	auto physicalDevices = s_VkInstance.enumeratePhysicalDevices();

	for (auto& device : physicalDevices) {
		auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();

		auto queueFamilyIndicies = findQueueFamilies(device, surface);

		std::set<int> uniqueQueueFamilyIndicies;
		if (queueFamilyIndicies.graphicsFamily != -1) {
			uniqueQueueFamilyIndicies.emplace(queueFamilyIndicies.graphicsFamily);
		}

		if (queueFamilyIndicies.presentFamily != -1) {
			uniqueQueueFamilyIndicies.emplace(queueFamilyIndicies.presentFamily);
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

// framebufferCount is a prefered minimum of buffers in the swapchain
SwapChain Device::createSwapChain(const Format& format, size_t framebufferCount, SwapChainPresentMode preferredPresentMode, Surface& surface)
{


	auto details = querySwapChainSupport(surface);

	auto swapFormat = chooseSwapSurfaceFormat(format, details.formats);

	auto presentMode = chooseSwapPresentMode(preferredPresentMode, details.presentmodes); 

	auto extent = chooseSwapChainExtend(surface.getWidth(), surface.getHeight(), details.capabilities);

	if (details.capabilities.maxImageCount > 0 &&
		framebufferCount > details.capabilities.maxImageCount) {
		framebufferCount = details.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo = {};
	createInfo.surface = surface;
	createInfo.minImageCount = framebufferCount;
	createInfo.imageFormat = swapFormat.format;
	createInfo.imageColorSpace = swapFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	QueueFamilyIndices indices = Device::findQueueFamilies(m_VkPhysicalDevice, surface);
	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily, indices.presentFamily
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0; //optional
		createInfo.pQueueFamilyIndices = nullptr; // optional
	}

	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	auto swapChain = m_VkDevice.createSwapchainKHR(createInfo);

	// Get colorbuffer images
	std::vector<vk::Image> images;
	images = m_VkDevice.getSwapchainImagesKHR(swapChain);
	auto actualFramebufferCount = images.size();

	// Get image resources for framebuffers
	std::vector<ImageResource> colorResources, depthResources;
	std::vector<Format> formatCandidates = {
		Format::eD32Sfloat, Format::eD32SfloatS8Uint, Format::eD24UnormS8Uint
	};

	for (auto i = 0; i < images.size(); ++i) {
		colorResources.emplace_back(ImageResource::createColorResource(images[i], m_VkDevice, swapFormat.format));
		depthResources.emplace_back(ImageResource::createDepthResource(m_VkPhysicalDevice, m_VkDevice, extent.width, extent.height, formatCandidates));
	}

	return SwapChain(m_VkDevice, swapChain, colorResources, depthResources, extent);
}


Device::QueueFamilyIndices Device::findQueueFamilies(const vk::PhysicalDevice & device, Surface& surface)
{
	int graphicsQueueFamily = -1;
	int presentQueueFamily = -1;

	auto queueFamilies = device.getQueueFamilyProperties();

	for (auto i = 0; i <  queueFamilies.size(); ++i) {
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


	return { graphicsQueueFamily, presentQueueFamily };
}

Device::SwapChainSupportDetails Device::querySwapChainSupport(Surface& surface)
{
	SwapChainSupportDetails details;
	details.capabilities = m_VkPhysicalDevice.getSurfaceCapabilitiesKHR(surface);

	details.formats = m_VkPhysicalDevice.getSurfaceFormatsKHR(surface);

	details.presentmodes = m_VkPhysicalDevice.getSurfacePresentModesKHR(surface);

	return details;
}

vk::SurfaceFormatKHR Device::chooseSwapSurfaceFormat(Format preferredFormat, std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	//In the case the surface has no preference
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
		return{ preferredFormat, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	// If we're not allowed to freely choose a format  
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == preferredFormat &&
			availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR Device::chooseSwapPresentMode(SwapChainPresentMode &preferredPresentMode, const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	vk::PresentModeKHR bestMode = preferredPresentMode;

	for (const auto& availablePresentMode : availablePresentModes) {

		if (availablePresentMode == preferredPresentMode) {
			return availablePresentMode;
		}

		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			bestMode = availablePresentMode;
		}
		else if (availablePresentMode == vk::PresentModeKHR::eImmediate && bestMode != vk::PresentModeKHR::eMailbox) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

vk::Extent2D Device::chooseSwapChainExtend(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR & capabilities)
{

	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}

	vk::Extent2D actualExtent = { width, height };

	//replacement for std::max(... , std::min(..., ...)) because macros
	auto minMaxImageExtendWidth = capabilities.maxImageExtent.width < actualExtent.width ? capabilities.maxImageExtent.width : actualExtent.width;
	actualExtent.width = capabilities.minImageExtent.width > minMaxImageExtendWidth ? capabilities.minImageExtent.width : minMaxImageExtendWidth;
		
	auto minMaxImageExtendHeight = capabilities.maxImageExtent.height < actualExtent.height ? capabilities.maxImageExtent.height : actualExtent.height;
	actualExtent.height = capabilities.minImageExtent.height > minMaxImageExtendHeight ? capabilities.minImageExtent.height : minMaxImageExtendHeight;

	return actualExtent;
}

vk::Instance Device::s_VkInstance;

