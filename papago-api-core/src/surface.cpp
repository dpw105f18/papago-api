#include "standard_header.hpp"
#include "surface.hpp"

#include <sstream>

Surface::operator vk::SurfaceKHR&()
{
	return *m_vkSurfaceKHR;
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

void Surface::checkInstanceLayers(const std::vector<const char*>& requiredLayers)
{
	auto layers = vk::enumerateInstanceLayerProperties();
	
	for(std::string layerName : requiredLayers)
	{
		if (!std::any_of(ITERATE(layers), [&layerName](const vk::LayerProperties& layerProperties)
		{
			return layerName == layerProperties.layerName;
		}))
		{
			std::stringstream stream;
			stream << "Required layer not supported: \"" << layerName << "\"";
			PAPAGO_ERROR(stream.str());
		}
	}
}

std::unique_ptr<ISurface> ISurface::createWin32Surface(size_t width, size_t height, HWND window)
{
	return std::make_unique<Surface>(width, height, window);
}

Surface::Surface(uint32_t width, uint32_t height, HWND hwnd) : ISurface(width, height)
{
	vk::ApplicationInfo appInfo("PapaGo-api", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	std::vector<const char*> surfaceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	};

#ifdef PAPAGO_USE_VALIDATION_LAYERS
	surfaceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif


	std::vector<const char*> requiredLayers = {};

#ifdef PAPAGO_USE_VALIDATION_LAYERS
	requiredLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif 

	checkInstanceLayers(requiredLayers);

	m_vkInstance = createInstanceUnique(vk::InstanceCreateInfo()
		.setEnabledLayerCount(requiredLayers.size())
		.setPpEnabledLayerNames(requiredLayers.data())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(surfaceExtensions.size())
		.setPpEnabledExtensionNames(surfaceExtensions.data()));

#ifdef PAPAGO_USE_VALIDATION_LAYERS
	m_debugReportCallback = m_vkInstance->createDebugReportCallbackEXTUnique(
		vk::DebugReportCallbackCreateInfoEXT()
			.setFlags(vk::DebugReportFlagBitsEXT() 
				| vk::DebugReportFlagBitsEXT::eWarning
				//| vk::DebugReportFlagBitsEXT::eInformation 
				| vk::DebugReportFlagBitsEXT::eError
				| vk::DebugReportFlagBitsEXT::ePerformanceWarning
				//| vk::DebugReportFlagBitsEXT::eDebug
			)
			.setPfnCallback(VkDebugCallback));
#endif

	auto surfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR()
		.setHwnd(hwnd)
		.setHinstance(GetModuleHandle(nullptr));

	m_vkSurfaceKHR = m_vkInstance->createWin32SurfaceKHRUnique(surfaceCreateInfo);
}
