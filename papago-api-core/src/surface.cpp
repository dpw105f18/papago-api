#include "standard_header.hpp"
#include "surface.hpp"

#include <sstream>

Surface::operator vk::SurfaceKHR&()
{
	return *m_vkSurfaceKHR;
}

uint32_t Surface::width() const {
	return m_width;
}

uint32_t Surface::height() const {
	return m_height;
}

#ifdef PAPAGO_USE_VALIDATION_LAYERS
#define GET_INSTANCE_PROCEDURE(instance, name) (PFN_##name)vkGetInstanceProcAddr(instance, #name);

VkResult vkCreateDebugReportCallbackEXT(
	VkInstance									instance,
	const VkDebugReportCallbackCreateInfoEXT*	pCreateInfo,
	const VkAllocationCallbacks*				pAllocator,
	VkDebugReportCallbackEXT*					pCallback)
{
	static auto func = GET_INSTANCE_PROCEDURE(instance, vkCreateDebugReportCallbackEXT);

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vkDestroyDebugReportCallbackEXT(
	VkInstance						instance,
	VkDebugReportCallbackEXT		callback,
	const VkAllocationCallbacks*	pAllocator)
{
	static auto func = GET_INSTANCE_PROCEDURE(instance, vkDestroyDebugReportCallbackEXT);

	if (func != nullptr) 
	{
		func(instance, callback, pAllocator);
	}
}

VkBool32 VKAPI_CALL VkDebugCallback(
	VkDebugReportFlagsEXT      flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t                   object,
	size_t                     location,
	int32_t                    messageCode,
	const char*                pLayerPrefix,
	const char*                pMessage,
	void*                      pUserData)
{
	switch(flags)
	{
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		std::cout << "[Info] "; break;
	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		std::cout << "[Error] "; break;
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
		std::cout << "[Warning] "; break;
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		std::cout << "[Performance Warning] "; break;
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		std::cout << "[Debug] "; break;
	default:
		std::cout << "[Unknown(" << flags << ")] "; break;
	}

	std::cout << pLayerPrefix << ": \"" << pMessage << "\"" << std::endl;

	return false;
}

#endif /* PAPAGO_USE_VALIDATION_LAYERS */

void Surface::checkInstanceLayers(std::vector<const char*> requiredLayers)
{
	auto layers = vk::enumerateInstanceLayerProperties();
	std::for_each(ITERATE(requiredLayers), [&layers](const std::string& layer_name)
	{
		if (!std::any_of(ITERATE(layers), [&layer_name](const vk::LayerProperties& layer_properties)
		{
			return layer_name == layer_properties.layerName;
		}))
		{
			std::stringstream stream;
			stream << "Required layer not supported: \"" << layer_name << "\"";
			throw std::runtime_error(stream.str());
		}
	});
}

Surface::Surface(uint32_t width, uint32_t height, HWND hwnd) : m_width(width), m_height(height)
{
	vk::ApplicationInfo appInfo("PapaGo-api", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	std::vector<const char*> surfaceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifdef PAPAGO_USE_VALIDATION_LAYERS
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif /* PAPAGO_USE_VALIDATION_LAYERS */
	};

	std::vector<const char*> requiredLayers = {
#ifdef PAPAGO_USE_VALIDATION_LAYERS
		"VK_LAYER_LUNARG_standard_validation"
#endif /* PAPAGO_USE_VALIDATION_LAYERS */
	};

	checkInstanceLayers(requiredLayers);

	m_vkInstance = createInstanceUnique(vk::InstanceCreateInfo()
		.setEnabledLayerCount(requiredLayers.size())
		.setPpEnabledLayerNames(requiredLayers.data())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(surfaceExtensions.size())
		.setPpEnabledExtensionNames(surfaceExtensions.data()));

#ifdef PAPAGO_USE_VALIDATION_LAYERS
	m_DebugReportCallback = m_vkInstance->createDebugReportCallbackEXTUnique(
		vk::DebugReportCallbackCreateInfoEXT()
			.setFlags(
				vk::DebugReportFlagBitsEXT::eWarning | 
				vk::DebugReportFlagBitsEXT::eInformation | 
				vk::DebugReportFlagBitsEXT::eError | 
				vk::DebugReportFlagBitsEXT::ePerformanceWarning|
				vk::DebugReportFlagBitsEXT::eDebug)
			.setPfnCallback(VkDebugCallback));
#endif

	m_vkSurfaceKHR = m_vkInstance->createWin32SurfaceKHRUnique(vk::Win32SurfaceCreateInfoKHR()
		.setHwnd(hwnd)
		.setHinstance(GetModuleHandle(nullptr)));
}
