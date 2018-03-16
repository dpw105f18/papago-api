#pragma once
#include "standard_header.hpp"
#include "device.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class Surface
{
public:
	operator vk::SurfaceKHR&();
private:
	//surface creation for Windows
	Surface(vk::SurfaceKHR surfaceKHR) : m_VkSurfaceKHR(surfaceKHR) {};
	vk::SurfaceKHR m_VkSurfaceKHR;

	friend Surface Device::createSurface(HWND&);
};