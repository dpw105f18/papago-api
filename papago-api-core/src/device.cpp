#include "standard_header.hpp"
#include <set>
#include "command_buffer.hpp"
#include "sub_command_buffer.hpp"
#include "surface.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "image_resource.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "render_pass.hpp"
#include "sampler.hpp"
#include "graphics_queue.hpp"
#include "shader_program.hpp"
#include "buffer_resource.hpp"
#include "parameter_block.hpp"

std::vector<std::unique_ptr<IDevice>> IDevice::enumerateDevices(ISurface & surface, const Features & features, const Extensions & extensions)
{
	// TODO: Support more features and extensions
	vk::PhysicalDeviceFeatures vkFeatures = {};
	vkFeatures.samplerAnisotropy = features.samplerAnisotropy;

	std::vector<const char *> vkExtensions;
	if (extensions.samplerMirrorClampToEdge) {
		vkExtensions.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
	}
	if (extensions.swapchain) {
		// Should this be forced on by default ?? - CW 2018-04-18
		vkExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	auto devices = Device::enumerateDevices((Surface&)surface, vkFeatures, vkExtensions);
	std::vector<std::unique_ptr<IDevice>> result;
	result.reserve(devices.size());
	for (auto& device : devices) {
		result.push_back(std::make_unique<Device>(std::move(device)));
	}
	return result;
}

//Provides a vector of devices with the given [features] and [extensions] enabled
std::vector<Device> Device::enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions)
{
	std::vector<const char*> enabledLayers;
#ifdef PAPAGO_USE_VALIDATION_LAYERS
	enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif 

	std::vector<Device> result;
	const float queuePriority = 1.0f;
	for (auto& physicalDevice : surface.m_vkInstance->enumeratePhysicalDevices()) {
		if (! isPhysicalDeviceSuitable(physicalDevice, surface, extensions)) {
			continue;
		}

		auto queueFamilyIndicies = findQueueFamilies(physicalDevice, surface);
		auto queueCreateInfos = createQueueCreateInfos(queueFamilyIndicies, queuePriority);

		auto logicalDevice = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo()
			.setEnabledExtensionCount(extensions.size())
			.setPpEnabledExtensionNames(extensions.data())
			.setPEnabledFeatures(&features)
			.setEnabledLayerCount(enabledLayers.size())
			.setPpEnabledLayerNames(enabledLayers.data())
			.setQueueCreateInfoCount(queueCreateInfos.size())
			.setPQueueCreateInfos(queueCreateInfos.data()));

		result.emplace_back(physicalDevice, logicalDevice, surface);
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
  
std::vector<vk::DeviceQueueCreateInfo> Device::createQueueCreateInfos(QueueFamilyIndices queueFamilyIndices, const float& queuePriority)
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
	const vk::PresentModeKHR& presentMode, 
	uint32_t queueFamilyIndices[],
	const bool preferMultiQueue) const
{
	auto createInfo = vk::SwapchainCreateInfoKHR()
		.setSurface(static_cast<vk::SurfaceKHR>(surface))
		.setMinImageCount(framebufferCount)
		.setImageFormat(swapFormat.format)
		.setImageColorSpace(swapFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		// IMPROVEMENT: All images are assumed to be transfer sources, so it can be downloaded. Is it more efficient to not do that? - CW 2018-04-23
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst) 
		.setPreTransform(capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE);

	auto indices = findQueueFamilies(m_vkPhysicalDevice, surface);
	queueFamilyIndices[0] = indices.graphicsFamily;
	queueFamilyIndices[1] = indices.presentFamily;

	
	if ((indices.graphicsFamily != indices.presentFamily) & preferMultiQueue) {
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

std::unique_ptr<IParameterBlock> Device::createParameterBlock(IRenderPass & renderPass, std::vector<ParameterBinding>& bindings)
{
	auto& internalRenderPass = dynamic_cast<RenderPass&>(renderPass);
	return std::make_unique<ParameterBlock>(m_vkDevice, internalRenderPass, bindings);
}


vk::UniqueRenderPass Device::createVkRenderpass(vk::Format colorFormat, vk::Format depthStencilFormat) const
{
	if (GetDepthStencilFlags(colorFormat) != DepthStencilFlags::eNone) {
		PAPAGO_ERROR("Supplied color format is a depth/stencil buffer format!");
	}

	if (GetDepthStencilFlags(depthStencilFormat) == DepthStencilFlags::eNone) {
		PAPAGO_ERROR("Supplied depth/stencil format is a color format!");
	}

	vk::AttachmentDescription colorAttachment;
	auto format = ImageResource::findSupportedFormat(
		m_vkPhysicalDevice,
		{ colorFormat }, 
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eColorAttachment
	);

	colorAttachment.setFormat(format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eGeneral)
		.setFinalLayout(vk::ImageLayout::eGeneral);

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	
	std::vector<vk::AttachmentDescription> attachments = { colorAttachment };
	

	vk::SubpassDescription subpass = {};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef);

	auto depthAttachmentRef = vk::AttachmentReference();

	format = ImageResource::findSupportedFormat(
		m_vkPhysicalDevice, 
		{ depthStencilFormat },
		vk::ImageTiling::eOptimal, 
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
			
	vk::AttachmentDescription depthAttachment;
	depthAttachment.setFormat(format)
		.setInitialLayout(vk::ImageLayout::eGeneral)
		.setFinalLayout(vk::ImageLayout::eGeneral);

	if (depthStencilFormat == vk::Format::eS8Uint)
	{
		depthAttachment.setLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStencilStoreOp(vk::AttachmentStoreOp::eStore);
	}
	else if(depthStencilFormat == vk::Format::eD32Sfloat)
	{
		depthAttachment.setLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	}
	else {
		depthAttachment.setLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStencilStoreOp(vk::AttachmentStoreOp::eStore);
	}
		

	attachments.push_back(depthAttachment);

	depthAttachmentRef = vk::AttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	subpass.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL);
	dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags())
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	return m_vkDevice->createRenderPassUnique(renderPassInfo);
}

vk::UniqueRenderPass Device::createVkRenderpass(vk::Format colorFormat) const
{
	if (GetDepthStencilFlags(colorFormat) != DepthStencilFlags::eNone) {
		PAPAGO_ERROR("Supplied color format is a depth/stencil buffer format!");
	}

	vk::AttachmentDescription colorAttachment;
	colorAttachment.setFormat(colorFormat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eGeneral)
		.setFinalLayout(vk::ImageLayout::eGeneral);

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

	std::vector<vk::AttachmentDescription> attachments = { colorAttachment };


	vk::SubpassDescription subpass = {};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef);

	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL);
	dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags())
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	return m_vkDevice->createRenderPassUnique(renderPassInfo);
}

// framebufferCount is a prefered minimum of buffers in the swapchain
std::unique_ptr<SwapChain> Device::createSwapChain(const vk::Format& format, size_t framebufferCount, vk::PresentModeKHR preferredPresentMode, bool preferMultiQueue = false)
{
	auto details = querySwapChainSupport(m_vkPhysicalDevice, m_surface);

	auto swapFormat = chooseSwapSurfaceFormat(format, details.formats);

	auto presentMode = chooseSwapPresentMode(preferredPresentMode, details.presentmodes); 

	auto extent = chooseSwapChainExtent(m_surface.getWidth(), m_surface.getHeight(), details.capabilities);

	if (details.capabilities.maxImageCount > 0 &&
		framebufferCount > details.capabilities.maxImageCount) {
		framebufferCount = details.capabilities.maxImageCount;
	}

	uint32_t queueFamilyIndices[2];
	auto createInfo = createSwapChainCreateInfo(m_surface, framebufferCount, swapFormat, extent, details.capabilities, presentMode, queueFamilyIndices, preferMultiQueue = false);

	auto swapChain =  m_vkDevice->createSwapchainKHRUnique(createInfo);

	// Get colorbuffer images
	auto images = m_vkDevice->getSwapchainImagesKHR(*swapChain);

	// Get image resources for framebuffers
	std::vector<ImageResource> colorResources, depthResources;


	auto resourceExtent = vk::Extent3D(extent.width, extent.height, 1);

	for (auto i = 0; i < images.size(); ++i) {
		colorResources.emplace_back(
			ImageResource::createColorResource(
				images[i],
				*this,
				swapFormat.format,
				resourceExtent));
	}

	return std::make_unique<SwapChain>(*this, swapChain, colorResources, extent);
}

std::unique_ptr<SwapChain> Device::createSwapChain(const vk::Format & colorFormat, vk::Format depthStencilFormat, size_t framebufferCount, vk::PresentModeKHR preferredPresentMode, bool preferMultiQueue = false)
{
	auto details = querySwapChainSupport(m_vkPhysicalDevice, m_surface);

	auto swapFormat = chooseSwapSurfaceFormat(colorFormat, details.formats);

	auto presentMode = chooseSwapPresentMode(preferredPresentMode, details.presentmodes);

	auto extent = chooseSwapChainExtent(m_surface.getWidth(), m_surface.getHeight(), details.capabilities);

	if (details.capabilities.maxImageCount > 0 &&
		framebufferCount > details.capabilities.maxImageCount) {
		framebufferCount = details.capabilities.maxImageCount;
	}

	uint32_t queueFamilyIndices[2];
	auto createInfo = createSwapChainCreateInfo(m_surface, framebufferCount, swapFormat, extent, details.capabilities, presentMode, queueFamilyIndices, preferMultiQueue);

	auto swapChain = m_vkDevice->createSwapchainKHRUnique(createInfo);

	// Get colorbuffer images
	auto images = m_vkDevice->getSwapchainImagesKHR(*swapChain);

	// Get image resources for framebuffers
	std::vector<ImageResource> colorResources, depthResources;
	std::vector<vk::Format> formatCandidates = {
		depthStencilFormat
	};

	auto resourceExtent = vk::Extent3D(extent.width, extent.height, 1);

	for (auto i = 0; i < images.size(); ++i) {
		colorResources.emplace_back(
			ImageResource::createColorResource(
				images[i],
				*this,
				swapFormat.format,
				resourceExtent));

		depthResources.emplace_back(
			ImageResource::createDepthResource(
				*this,
				resourceExtent,
				formatCandidates));
	}

	return std::make_unique<SwapChain>(*this, swapChain, colorResources, depthResources, extent);
}

std::unique_ptr<ISwapchain> Device::createSwapChain(Format format, size_t framebufferCount, PresentMode preferredPesentMode, bool preferMultiQueue = false)
{
	vk::PresentModeKHR vkPreferredPresentMode;
	switch (preferredPesentMode)
	{
	case IDevice::PresentMode::eMailbox:
		vkPreferredPresentMode = vk::PresentModeKHR::eMailbox;
		break;
	default:
		PAPAGO_ERROR("Unknown presentmode");
		break;
	}
	return createSwapChain(to_vulkan_format(format), framebufferCount, vkPreferredPresentMode, preferMultiQueue);
}

std::unique_ptr<ISwapchain> Device::createSwapChain(Format colorFormat, Format depthStencilFormat, size_t framebufferCount, PresentMode preferredPresentMode, bool preferMultiQueue = false)
{
	vk::PresentModeKHR vkPreferredPresentMode;
	switch (preferredPresentMode)
	{
	case IDevice::PresentMode::eMailbox:
		vkPreferredPresentMode = vk::PresentModeKHR::eMailbox;
		break;
	default:
		PAPAGO_ERROR("Unknown presentmode");
		break;
	}
	return createSwapChain(to_vulkan_format(colorFormat), to_vulkan_format(depthStencilFormat), framebufferCount, vkPreferredPresentMode, preferMultiQueue);
}

std::unique_ptr<IGraphicsQueue> Device::createGraphicsQueue()
{
	auto queueFamilyIndices = findQueueFamilies(m_vkPhysicalDevice, m_surface);
	return std::make_unique<GraphicsQueue>(
		*this, 
		queueFamilyIndices.graphicsFamily, 
		queueFamilyIndices.presentFamily 
	);
}

std::unique_ptr<ICommandBuffer> Device::createCommandBuffer()
{
	auto queueFamilyIndices = findQueueFamilies(m_vkPhysicalDevice, m_surface);
	return std::make_unique<CommandBuffer>(
		m_vkDevice, 
		queueFamilyIndices.graphicsFamily);
}

std::unique_ptr<ISubCommandBuffer> Device::createSubCommandBuffer()
{
	auto queueFamilyIndex = findQueueFamilies(m_vkPhysicalDevice, m_surface).graphicsFamily;
	return std::make_unique<SubCommandBuffer>(m_vkDevice, queueFamilyIndex);
}

std::unique_ptr<IDynamicBufferResource> Device::createDynamicUniformBuffer(size_t objectSize, int objectCount)
{
	auto dynamic_alligment = objectSize;
	const auto properties = m_vkPhysicalDevice.getProperties();
	const auto allignment = properties.limits.minUniformBufferOffsetAlignment;

	if(allignment > 0)
	{
		dynamic_alligment = (dynamic_alligment + allignment - 1) & ~(allignment - 1);
	}

	const auto buffer_size = dynamic_alligment * objectCount;
	auto buffer = BufferResource::createBufferResource(
		m_vkPhysicalDevice, 
		m_vkDevice, 
		buffer_size, 
		vk::BufferUsageFlagBits::eUniformBuffer, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	return std::make_unique<DynamicBufferResource>(std::move(buffer), dynamic_alligment, objectCount);
}

/* //TODO: delete if not used!
//TODO: remove 2D from method name and let dimension be determined by the (number of) arguments? -AM
ImageResource Device::createTexture2D(uint32_t width, uint32_t height, vk::Format format )
{
	vk::Extent3D extent = { width, height, 1 };
	vk::ImageCreateInfo info;
	info.setImageType(vk::ImageType::e2D)
		.setExtent(extent)
		.setFormat(format)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setUsage(vk::ImageUsageFlagBits::eTransferDst 
			| vk::ImageUsageFlagBits::eSampled 
			| vk::ImageUsageFlagBits::eColorAttachment
			| vk::ImageUsageFlagBits::eTransferSrc);

	auto image = m_vkDevice->createImage(info);
	auto memoryRequirements = m_vkDevice->getImageMemoryRequirements(image);
	return ImageResource(image, *this, vk::ImageAspectFlagBits::eColor, format, extent, memoryRequirements);
}
*/

std::unique_ptr<IShaderProgram> Device::createShaderProgram(IVertexShader &vertexShader, IFragmentShader &fragmentShader)
{
	return std::make_unique<ShaderProgram>(m_vkDevice, (VertexShader&)vertexShader, (FragmentShader&)fragmentShader);
}

std::unique_ptr<IBufferResource> Device::createUniformBuffer(size_t size)
{
	return BufferResource::createBufferResource(
		m_vkPhysicalDevice,
		m_vkDevice,
		size,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

std::unique_ptr<IBufferResource> Device::createVertexBufferInternal(std::vector<char>& data)
{
	size_t bufferSize = data.size();
	auto buffer = BufferResource::createBufferResource(
		m_vkPhysicalDevice,
		m_vkDevice,
		bufferSize,
		vk::BufferUsageFlagBits::eVertexBuffer,
		// TODO: Convert to device local memory when command pool and buffers are ready
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	buffer->upload(data);
	return buffer;
}

std::unique_ptr<IBufferResource> Device::createIndexBufferInternal(std::vector<char>& data, BufferResourceElementType type)
{
	size_t bufferSize = data.size();
	auto buffer = BufferResource::createBufferResource(
		m_vkPhysicalDevice,
		m_vkDevice,
		bufferSize,
		vk::BufferUsageFlagBits::eIndexBuffer,
		// TODO: Convert to device local memory when command pool and buffers are ready
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		type);

	buffer->upload(data);
	return buffer;
}

void Device::waitIdle()
{
	m_vkDevice->waitIdle();
}

const vk::UniqueDevice& Device::getVkDevice() const
{
	return m_vkDevice;
}

const vk::PhysicalDevice& Device::getVkPhysicalDevice() const
{
	return m_vkPhysicalDevice;
}

std::unique_ptr<IRenderPass> Device::createRenderPass(IShaderProgram & program, uint32_t width, uint32_t height, Format colorFormat)
{
	auto vkPass = createVkRenderpass(to_vulkan_format(colorFormat));
	return std::make_unique<RenderPass>(
		m_vkDevice,
		vkPass,
		static_cast<ShaderProgram&>(program),
		vk::Extent2D{ width, height },
		DepthStencilFlags::eNone);
}

std::unique_ptr<IRenderPass> Device::createRenderPass(IShaderProgram & program, uint32_t width, uint32_t height, Format colorFormat, Format depthStencilFormat)
{
	auto& innerProgram = dynamic_cast<ShaderProgram&>(program);
	auto renderPasses = std::vector<vk::UniqueRenderPass>();
	auto vkPass = createVkRenderpass(to_vulkan_format(colorFormat), to_vulkan_format(depthStencilFormat));
	return std::make_unique<RenderPass>(
		m_vkDevice,
		vkPass,
		static_cast<ShaderProgram&>(program),
		vk::Extent2D{ width, height },
		GetDepthStencilFlags(to_vulkan_format(depthStencilFormat)));
}

std::unique_ptr<ISampler> Device::createTextureSampler1D(Filter magFilter, Filter minFilter, TextureWrapMode modeU)
{
	auto sampler = std::make_unique<Sampler>(SamplerD::e1D);
	sampler->setMagFilter(to_vulkan_filter(magFilter))
		.setMinFilter(to_vulkan_filter(minFilter))
		.setTextureWrapU(to_vulkan_address_mode(modeU));

	sampler->m_vkTextureSampler = m_vkDevice->createSamplerUnique(sampler->m_vkSamplerCreateInfo);

	return sampler;
}

std::unique_ptr<ISampler> Device::createTextureSampler2D(Filter magFilter, Filter minFilter, TextureWrapMode modeU, TextureWrapMode modeV)
{
	auto sampler = std::make_unique<Sampler>(SamplerD::e2D);
	sampler->setMagFilter(to_vulkan_filter(magFilter))
		.setMinFilter(to_vulkan_filter(minFilter))
		.setTextureWrapU(to_vulkan_address_mode(modeU))
		.setTextureWrapV(to_vulkan_address_mode(modeV));

	sampler->m_vkTextureSampler = m_vkDevice->createSamplerUnique(sampler->m_vkSamplerCreateInfo);

	return sampler;
}

std::unique_ptr<ISampler> Device::createTextureSampler3D(Filter magFilter, Filter minFilter, TextureWrapMode modeU, TextureWrapMode modeV, TextureWrapMode modeW)
{
	auto sampler = std::make_unique<Sampler>(SamplerD::e3D);
	sampler->setMagFilter(to_vulkan_filter(magFilter))
		.setMinFilter(to_vulkan_filter(minFilter))
		.setTextureWrapU(to_vulkan_address_mode(modeU))
		.setTextureWrapV(to_vulkan_address_mode(modeV))
		.setTextureWrapW(to_vulkan_address_mode(modeW));

	sampler->m_vkTextureSampler = m_vkDevice->createSamplerUnique(sampler->m_vkSamplerCreateInfo);

	return sampler;
}

std::unique_ptr<IImageResource> Device::createTexture2D(size_t width, size_t height, Format format)
{
	vk::Extent3D extent = { uint32_t(width), uint32_t(height), 1 };
	vk::ImageCreateInfo info;
	info.setImageType(vk::ImageType::e2D)
		.setExtent(extent)
		.setFormat(to_vulkan_format(format))
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment);

	auto image = m_vkDevice->createImage(info);
	auto memoryRequirements = m_vkDevice->getImageMemoryRequirements(image);
	return std::make_unique<ImageResource>(image, *this, vk::ImageAspectFlagBits::eColor, to_vulkan_format(format), extent, memoryRequirements);
}

std::unique_ptr<IImageResource> Device::createDepthTexture2D(uint32_t width, uint32_t height, Format format)
{
	return std::make_unique<ImageResource>(ImageResource::createDepthResource(*this, { width, height, 1 }, { to_vulkan_format(format) }));
}

Device::Device(vk::PhysicalDevice physicalDevice, vk::UniqueDevice &device, Surface &surface)
	: m_vkPhysicalDevice(physicalDevice)
	, m_vkDevice(std::move(device))
	, m_surface(surface)
	, m_internalCommandBuffer(CommandBuffer{ m_vkDevice, findQueueFamilies(physicalDevice, surface).graphicsFamily})
{
	m_vkInternalQueue = m_vkDevice->getQueue(findQueueFamilies(physicalDevice, surface).graphicsFamily, 0);
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

vk::SurfaceFormatKHR Device::chooseSwapSurfaceFormat(vk::Format preferredFormat, std::vector<vk::SurfaceFormatKHR>& availableFormats)
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

vk::PresentModeKHR Device::chooseSwapPresentMode(vk::PresentModeKHR preferredPresentMode, const std::vector<vk::PresentModeKHR>& availablePresentModes)
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

vk::Extent2D Device::chooseSwapChainExtent(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR & capabilities)
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