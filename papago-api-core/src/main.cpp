#include "standard_header.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "surface.hpp"
#include <WinUser.h>

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			if (MessageBox(0, "Are you a quitter?", "QUEST: A true quitter", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

HWND StartWindow(size_t width, size_t height)
{
	//******************** CREATE WINDOW ***************************
	auto hInstance = GetModuleHandle(nullptr);
	auto windowName = "test window name";
	auto windowClassName = "testWindowClassName";


	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 2);
	wc.lpszClassName = windowClassName;
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		throw std::runtime_error("Could not register window class");
	}

	HWND hwnd;
	try {

		hwnd = CreateWindowEx(0,
			windowClassName,
			windowName,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width, height,
			nullptr, nullptr,
			hInstance, nullptr);
		if (!hwnd)
		{
			throw new std::runtime_error("Could not create window");
		}
	}
	catch (const std::runtime_error& e)
	{
		auto errorCode = GetLastError();
		std::cout << "error code: " << errorCode << "\nWhat: " << e.what() << std::endl;
		throw e;
	}
	catch (...)
	{
		auto errorCode = GetLastError();
		std::cout << "error code: " << errorCode << "\nWhat: " << "..." << std::endl;
	}

	//if everything went well, show the window.
	ShowWindow(hwnd, true);
	UpdateWindow(hwnd);

	return hwnd;
}

int main()
{
	/*
	std::cout << "Hello, world!" << std::endl;
	auto instance = vk::createInstance(vk::InstanceCreateInfo());
	std::cout << "Created a vulkan instance." << std::endl;
	char ch;
	std::cin >> ch;

	instance.destroy();
	*/
	size_t winHeight = 800;
	size_t winWidth = 600;
	auto hwnd = StartWindow(winWidth, winHeight);


	auto surface = Device::createSurface(winWidth, winHeight, hwnd);
	vk::PhysicalDeviceFeatures features = {};
	features.samplerAnisotropy = VK_TRUE;
	auto devices = Device::enumerateDevices(surface, features, { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
	auto device = devices[0];
	auto swapChain = device.createSwapChain(Format::eR8G8B8Unorm, 3, SwapChainPresentMode::eMailbox, surface);

	std::system("PAUSE");
}