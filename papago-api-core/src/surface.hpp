#pragma once
#include "standard_header.hpp"
#include "device.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX	//<--- we don't want to define min and max macros from windows.h - BUT THAT STILL HAPPENS FOR SOME REASON!!????
#include <Windows.h>

class Surface
{
public:
	operator vk::SurfaceKHR&();
	uint32_t getWidth();
	uint32_t getHeight();
private:
	//surface creation for Windows
	Surface(uint32_t width, uint32_t height, vk::SurfaceKHR surfaceKHR) : width(width), height(height), m_VkSurfaceKHR(surfaceKHR) {};
	vk::SurfaceKHR m_VkSurfaceKHR;
	
	uint32_t width, height;

	friend Surface Device::createSurface(size_t width, size_t height, HWND&);
};