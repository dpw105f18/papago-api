#include "standard_header.hpp"
#include "surface.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "image_resource.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include <set>
#include "render_pass.hpp"

//Provides a vector of devices with the given [features] and [extensions] enabled
std::vector<Device> Device::enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions)
{
	std::vector<Device> result;

	for (auto& physicalDevice : surface.m_vkInstance->enumeratePhysicalDevices()) {
		if (! isPhysicalDeviceSuitable(physicalDevice, surface, extensions)) {
			continue;
		}

		auto queueFamilyIndicies = findQueueFamilies(physicalDevice, surface);
		auto queueCreateInfos = createQueueCreateInfos(queueFamilyIndicies);

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

// Tries to keep graphics and present families on separate physical queues
Device::QueueFamilyIndices Device::findQueueFamilies(const vk::PhysicalDevice & device, Surface& surface)
{
	int graphicsQueueFamily = QueueFamilyIndices::NOT_FOUND();
	int presentQueueFamily = QueueFamilyIndices::NOT_FOUND();

	auto queueFamilies = device.getQueueFamilyProperties();

	for (auto i = 0; i < queueFamilies.size(); ++i) {
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
				device.getSurfaceSupportKHR(i, static_cast<vk::SurfaceKHR>(surface))) {
				presentQueueFamily = i;
			}
		}
	}

	return { graphicsQueueFamily, presentQueueFamily };
}
  
std::vector<vk::DeviceQueueCreateInfo> Device::createQueueCreateInfos(QueueFamilyIndices queueFamilyIndices)
{
	std::set<int> uniqueQueueFamilyIndicies;
	if (queueFamilyIndices.hasGraphicsFamily()) {
		uniqueQueueFamilyIndicies.emplace(queueFamilyIndices.graphicsFamily);
	}

	if (queueFamilyIndices.hasPresentFamily()) {
		uniqueQueueFamilyIndicies.emplace(queueFamilyIndices.presentFamily);
	}

	auto queueCreateInfos = std::vector<vk::DeviceQueueCreateInfo>();
	for (auto queueFamilyIndex : uniqueQueueFamilyIndicies) {
		const float queuePriority = 1.0f;	//<-- TODO: should this be setable somehow?

		queueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
			.setQueueCount(1)					//<-- TODO: setable?
			.setPQueuePriorities(&queuePriority)
			.setQueueFamilyIndex(queueFamilyIndex));
	}

	return queueCreateInfos;
}

vk::SwapchainCreateInfoKHR Device::createSwapChainCreateInfo(
	Surface &surface, 
	const size_t &framebufferCount, 
	const vk::SurfaceFormatKHR &swapFormat, 
	const vk::Extent2D &extent,
	const vk::SurfaceCapabilitiesKHR& capabilities, 
	const vk::PresentModeKHR& presentMode) const
{
	auto createInfo = vk::SwapchainCreateInfoKHR()
		.setSurface(static_cast<vk::SurfaceKHR>(surface))
		.setMinImageCount(framebufferCount)
		.setImageFormat(swapFormat.format)
		.setImageColorSpace(swapFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setPreTransform(capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE);

	auto indices = findQueueFamilies(m_vkPhysicalDevice, surface);
	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily, indices.presentFamily
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(queueFamilyIndices);
	}
	else {
		createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndexCount(0)
			.setPQueueFamilyIndices(nullptr);
	}

	return createInfo;
}

bool Device::isPhysicalDeviceSuitable(const vk::PhysicalDevice & physicalDevice, Surface& surface, const std::vector<const char*>& extensions)
{
	auto queueFamilyIndicies = findQueueFamilies(physicalDevice, surface);

	bool extensionsSupported = areExtensionsSupported(physicalDevice, extensions);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		auto swapChainSupport = querySwapChainSupport(physicalDevice, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentmodes.empty();
	}

	return queueFamilyIndicies.isComplete() && extensionsSupported && swapChainAdequate;
}

bool Device::areExtensionsSupported(const vk::PhysicalDevice & physicalDevice, const std::vector<const char*>& extensions)
{
	std::set<std::string> requiredExtensions(ITERATE(extensions));

	std::vector<vk::ExtensionProperties> avaliableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	for (const auto& extension : avaliableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

// framebufferCount is a prefered minimum of buffers in the swapchain
SwapChain Device::createSwapChain(const Format& format, size_t framebufferCount, SwapChainPresentMode preferredPresentMode, Surface& surface)
{
	auto details = querySwapChainSupport(m_vkPhysicalDevice, surface);

	auto swapFormat = chooseSwapSurfaceFormat(format, details.formats);

	auto presentMode = chooseSwapPresentMode(preferredPresentMode, details.presentmodes); 

	auto extent = chooseSwapChainExtend(surface.getWidth(), surface.getHeight(), details.capabilities);

	if (details.capabilities.maxImageCount > 0 &&
		framebufferCount > details.capabilities.maxImageCount) {
		framebufferCount = details.capabilities.maxImageCount;
	}

	auto createInfo = createSwapChainCreateInfo(surface, framebufferCount, swapFormat, extent, details.capabilities, presentMode);

	auto swapChain =  m_vkDevice->createSwapchainKHRUnique(createInfo);

	// Get colorbuffer images
	auto images = m_vkDevice->getSwapchainImagesKHR(*swapChain);

	// Get image resources for framebuffers
	std::vector<ImageResource> colorResources, depthResources;
	std::vector<Format> formatCandidates = {
		Format::eD32Sfloat, 
		Format::eD32SfloatS8Uint, 
		Format::eD24UnormS8Uint
	};

	for (auto i = 0; i < images.size(); ++i) {
		colorResources.emplace_back(
			ImageResource::createColorResource(
				images[i], 
				m_vkDevice, 
				swapFormat.format));

		depthResources.emplace_back(
			ImageResource::createDepthResource(
				m_vkPhysicalDevice, m_vkDevice, 
				extent.width, extent.height, 
				formatCandidates));
	}

	return SwapChain(m_vkDevice, swapChain, colorResources, depthResources, extent);

}

VertexShader Device::createVertexShader(const std::string & filePath, const std::string & entryPoint) const {
	return VertexShader(m_vkDevice, filePath, entryPoint);
}

FragmentShader Device::createFragmentShader(const std::string & filePath, const std::string & entryPoint) const {
	return FragmentShader(m_vkDevice, filePath, entryPoint);
}

RenderPass Device::createRenderPass(VertexShader &vertexShader, FragmentShader &fragmentShader, const SwapChain &swapChain) const
{
	// TODO: Dangerous hacking, fix this by adding error handling instead of expecting there always being data available.
	auto extent = swapChain.m_colorResources[0].m_VkCreateInfo.extent;
	auto format = swapChain.m_colorResources[0].m_Format;

	return RenderPass(m_vkDevice, vertexShader, fragmentShader, { extent.width, extent.height }, format);
}




Device::Device(vk::PhysicalDevice physicalDevice, vk::UniqueDevice &device) 
	: m_vkPhysicalDevice(physicalDevice)
	, m_vkDevice(std::move(device))
{

}

Device::SwapChainSupportDetails Device::querySwapChainSupport(const vk::PhysicalDevice& physicalDevice, Surface& surface) 
{
	auto innerSurface = static_cast<vk::SurfaceKHR>(surface);

	SwapChainSupportDetails details;
	details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(innerSurface);
	details.formats = physicalDevice.getSurfaceFormatsKHR(innerSurface);
	details.presentmodes = physicalDevice.getSurfacePresentModesKHR(innerSurface);

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
	// FIFO is guarrenteed to be supported according to the Vulkan Specification
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


