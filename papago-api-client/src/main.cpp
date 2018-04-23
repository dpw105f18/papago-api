#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <fstream>

#include "ishader.hpp"
#include "parser.hpp"
#include "isampler.hpp"
#include "isurface.hpp"
#include "iswapchain.hpp"
#include "iimage_resource.hpp"
#include "ibuffer_resource.hpp"
#include "idevice.hpp"
#include "api_enums.hpp"

struct vec2
{
	float x, y;
};

struct vec3
{
	float x, y, z;
};

struct Vertex
{
	vec2 m_position;
};


LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			if (MessageBox(nullptr,L"Are you sure you want to quit?", L"Quit", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				DestroyWindow(hwnd);
			}
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
	auto hInstance = GetModuleHandle(nullptr);
	auto windowName = L"test window name";
	auto windowClassName = L"testWindowClassName";


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

		if (!hwnd) throw std::runtime_error("Could not create window");

		//if everything went well, show the window.
		ShowWindow(hwnd, true);
		UpdateWindow(hwnd);

		return hwnd;
	}
	catch (const std::runtime_error& e)
	{
		auto errorCode = GetLastError();
		std::cout << "error code: " << errorCode << "\nWhat: " << e.what() << std::endl;
		throw;
	}
	catch (...)
	{
		auto errorCode = GetLastError();
		std::cout << "error code: " << errorCode << "\nWhat: " << "..." << std::endl;
	}
	return nullptr;
}

struct UniformBufferObject {};

std::unique_ptr<IImageResource> createTexture(IDevice& device) {
	int texWidth, texHeight, texChannels;
	auto pixels = stbi_load("textures/eldorado.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw new std::runtime_error("Failed to load texture image!");
	}

	auto input = std::vector<char>();
	input.resize(texWidth * texHeight * 4);
	memcpy(input.data(), pixels, input.size());
	stbi_image_free(pixels);

	auto imageResource = device.createTexture2D(texWidth, texHeight, Format::eR8G8B8A8Unorm);
	imageResource->upload(input);

	return imageResource;
}


std::string readFile(const std::string& file_path) {
	std::ifstream stream(file_path, std::ios::ate);
	auto size = stream.tellg();
	stream.seekg(0);
	std::string result = std::string(size, '\0');
	stream.read(&result[0], size);
	return result;
}

int main()
{
	{
		auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin32/glslangValidator.exe");

		size_t winWidth = 800;
		size_t winHeight = 600;
		auto hwnd = StartWindow(winWidth, winHeight);
		auto surface = ISurface::createWin32Surface(winWidth, winHeight, hwnd);
		IDevice::Features features;
		features.samplerAnisotropy = true;
		IDevice::Extensions extensions;
		extensions.swapchain = true;
		extensions.samplerMirrorClampToEdge = true;
		auto devices = IDevice::enumerateDevices(*surface, features, extensions);
		auto& device = devices[0];
		auto swapChain = device->createSwapChain(Format::eR8G8B8Unorm, 3, IDevice::PresentMode::eMailbox);
		auto vertexBuffer = device->createVertexBuffer(std::vector<Vertex>{
			{ 0.3f, -1.0f },
			{ 0.52f, 0.4f },
			{ -0.5f, 0.5f },
			{ -0.3f, 1.0f },
			{ -0.52f, -0.4f },
			{ 0.5f, -0.5f },
		});
		auto indexBuffer = device->createIndexBuffer(std::vector<uint16_t>{
			0, 1, 2
		});

		auto uniformBuffer = device->createUniformBuffer(sizeof(UniformBufferObject));

		auto bigUniform = device->createUniformBuffer(1000);

		std::vector<char> bigData(1000);
		for (auto i = 0; i < 1000; ++i) {
			bigData[i] = i % 256;
		}

		auto sampler3D = device->createTextureSampler3D(Filter::eNearest, Filter::eNearest, TextureWrapMode::eClampToBorder, TextureWrapMode::eClampToEdge, TextureWrapMode::eRepeat);
		auto sampler2D = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirrorClampToEdge);
		auto sampler1D = device->createTextureSampler1D(Filter::eNearest, Filter::eNearest, TextureWrapMode::eRepeat);

		/*
		auto image = createTexture(*device);

		bigUniform.upload(bigData);

		auto dlData = bigUniform.download();
		*/
		auto vertexShader = parser.compileVertexShader(readFile("shader/stupidVert.vert"), "main");
		vertexShader = parser.compileVertexShader(readFile("shader/colorVert.vert"), "main");
		auto fragmentShader = parser.compileFragmentShader(readFile("shader/stupidFrag.frag"), "main");
		fragmentShader = parser.compileFragmentShader(readFile("shader/colorFrag.frag"), "main");
		/*
		auto program = device.createShaderProgram(*vertexShader, *fragmentShader);

		auto renderPass = device.createRenderPass(program, swapChain);

		auto graphicsQueue = device.createGraphicsQueue(swapChain);
		size_t frameNo = 0;	//<-- for debugging



		while (true)
		{
			MSG msg;
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT) {
					break;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				auto cmd = device.createCommandBuffer(Usage::eReset);
				cmd.begin(renderPass, swapChain, graphicsQueue.getCurrentFrameIndex());
				cmd.setInput(vertexBuffer);
				cmd.setUniform("texSampler", image, sampler2D);
				cmd.setIndexBuffer(indexBuffer);
				cmd.drawIndexed(3); // TODO: Create a way to get this from the index buffer
				//cmd.drawInstanced(vertexBuffer.getSize(), 1, 0, 0);
				cmd.end();
				std::vector<CommandBuffer> commandBuffers;
				commandBuffers.push_back(std::move(cmd));
				graphicsQueue.present(commandBuffers);
				frameNo++;
				graphicsQueue.Wait();
			}
		}
		device.waitIdle();
		*/
	}
	std::cout << "Press enter to continue...";
	std::cin.ignore();
}
