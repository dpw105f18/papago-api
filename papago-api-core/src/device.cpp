#include "standard_header.hpp"
#include "surface.hpp"
#include "device.hpp"
#include <set>
#include "swap_chain.hpp"

//Provides a vector of devices with the given [features] and [extensions] enabled
std::vector<Device> Device::enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions)
{
	auto physicalDevices = surface.m_vkInstance->enumeratePhysicalDevices();
	std::vector<Device> result;
	result.reserve(physicalDevices.size());

	for (auto& physicalDevice : physicalDevices) 
	{
		auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();

		auto queueFamilyIndicies = findQueueFamilies(physicalDevice, surface);

		std::set<int> uniqueQueueFamilyIndicies;
		if (queueFamilyIndicies.graphicsFamily != -1)
		{
			uniqueQueueFamilyIndicies.emplace(queueFamilyIndicies.graphicsFamily);
		}

		if (queueFamilyIndicies.presentFamily != -1) 
		{
			uniqueQueueFamilyIndicies.emplace(queueFamilyIndicies.presentFamily);
		}

		const float queuePriority = 1.0f;	//<-- TODO: should this be setable somehow?
		for (auto qf : uniqueQueueFamilyIndicies) 
		{
			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
				.setQueueCount(1)					//<-- TODO: setable?
				.setPQueuePriorities(&queuePriority)
				.setQueueFamilyIndex(qf));
		}

		auto logicalDevice = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo()
			.setEnabledExtensionCount(extensions.size())
			.setPpEnabledExtensionNames(extensions.data())
			.setPEnabledFeatures(&features)
			.setEnabledLayerCount(0)
			.setQueueCreateInfoCount(queueCreateInfos.size())
			.setPQueueCreateInfos(queueCreateInfos.data()));

		result.push_back(Device(physicalDevice, logicalDevice));
	}
	return result;
}

// framebufferCount is a prefered minimum of buffers in the swapchain
SwapChain Device::createSwapChain(const Format& format, size_t framebufferCount, SwapChainPresentMode preferredPresentMode, Surface& surface)
{
	auto details = querySwapChainSupport(surface);

	auto swapFormat = chooseSwapSurfaceFormat(format, details.formats);

	auto presentMode = chooseSwapPresentMode(preferredPresentMode, details.presentmodes); 

	auto extent = chooseSwapChainExtend(surface.width(), surface.height(), details.capabilities);

	if (details.capabilities.maxImageCount > 0 &&
		framebufferCount > details.capabilities.maxImageCount) {
		framebufferCount = details.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo = {};
	createInfo.surface = static_cast<vk::SurfaceKHR>(surface);
	createInfo.minImageCount = framebufferCount;
	createInfo.imageFormat = swapFormat.format;
	createInfo.imageColorSpace = swapFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	QueueFamilyIndices indices = findQueueFamilies(m_vkPhysicalDevice, surface);
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

	auto swapChain = m_vkDevice->createSwapchainKHRUnique(createInfo);

	return SwapChain(m_vkDevice, swapChain, swapFormat.format, extent);
}

Device::QueueFamilyIndices Device::findQueueFamilies(const vk::PhysicalDevice & device, Surface& surface)
{
	int graphicsQueueFamily = QueueFamilyIndices::NOT_FOUND();
	int presentQueueFamily = QueueFamilyIndices::NOT_FOUND();

	auto queueFamilies = device.getQueueFamilyProperties();

	for (auto i = 0; i <  queueFamilies.size(); ++i) {
		auto queueFamily = queueFamilies.at(i);

		if (queueFamily.queueCount > 0) {
			if ((graphicsQueueFamily == QueueFamilyIndices::NOT_FOUND() || 
				graphicsQueueFamily == presentQueueFamily) && 
				queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				graphicsQueueFamily = i;
			}

			if ((presentQueueFamily == QueueFamilyIndices::NOT_FOUND() || 
				presentQueueFamily == graphicsQueueFamily) && 
				device.getSurfaceSupportKHR(i, static_cast<vk::SurfaceKHR>(surface)))
			{
				presentQueueFamily = i;
			}
		}
	}


	return { graphicsQueueFamily, presentQueueFamily };
}

Device::Device(vk::PhysicalDevice physicalDevice, vk::UniqueDevice& device): m_vkPhysicalDevice(physicalDevice), m_vkDevice(std::move(device)) {}

Device::SwapChainSupportDetails Device::querySwapChainSupport(Surface& surface) const
{
	auto innerSurface = static_cast<vk::SurfaceKHR>(surface);

	SwapChainSupportDetails details;
	details.capabilities = m_vkPhysicalDevice.getSurfaceCapabilitiesKHR(innerSurface);
	details.formats = m_vkPhysicalDevice.getSurfaceFormatsKHR(innerSurface);
	details.presentmodes = m_vkPhysicalDevice.getSurfacePresentModesKHR(innerSurface);

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
	auto bestMode = vk::PresentModeKHR::eFifo;

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

	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	vk::Extent2D actualExtent = { width, height };

	actualExtent.width = std::max(
		capabilities.minImageExtent.width, 
		std::min(
			capabilities.maxImageExtent.width, 
			actualExtent.width));
	
	actualExtent.height = std::max(
		capabilities.minImageExtent.height, 
		std::min(
			capabilities.maxImageExtent.height, 
			actualExtent.height));

	return actualExtent;
}