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
#include "icommand_buffer.hpp"
#include "ishader_program.hpp"
#include "icommand_buffer.hpp"
#include "igraphics_queue.hpp"
#include "irender_pass.hpp"
#include "idevice.hpp"
#include "api_enums.hpp"

struct vec2
{
	float x, y;
};

union vec3
{
	struct { float x, y, z; }; // Positions
	struct { float r, g, b; }; // Colors
	struct { float u, v, w; }; // Texture Maps
};

struct Vertex
{
	vec3 m_position;
	vec2 m_textureCoordinate;
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
		auto hwnd = StartWindow(800, 600);
		auto surface = ISurface::createWin32Surface(800, 600, hwnd);
		IDevice::Features features;
		features.samplerAnisotropy = true;
		IDevice::Extensions extensions;
		extensions.swapchain = true;
		extensions.samplerMirrorClampToEdge = true;
		auto devices = IDevice::enumerateDevices(*surface, features, extensions);
		auto& device = devices[0];

		auto stupidVertexBuffer = device->createVertexBuffer(std::vector<Vertex>{
			{ { -0.5, -0.5, 0.5 }, { 0.0, 0.0 } },
			{ { -0.5,  0.5, 0.5 }, { 0.0, 1.0 } },
			{ {  0.5,  0.5, 0.5 }, { 1.0, 1.0 } },
			{ {  0.5, -0.5, 0.5 }, { 1.0, 0.0 } }
		});
		auto vertexBuffer = device->createVertexBuffer(std::vector<vec3>{
			{-0.5, -0.5, 0.5 },
			{-0.5,  0.5, 0.5 },
			{ 0.5,  0.5, 0.5 },
			{ 0.5, -0.5, 0.5 }
		});
		auto indexBuffer = device->createIndexBuffer(std::vector<uint16_t>{
			0, 1, 2,
			0, 2, 3
		});

		auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin32/glslangValidator.exe");
		auto swapchain = device->createSwapChain(Format::eR8G8B8A8Unorm, Format::eD32SfloatS8Uint, 3, IDevice::PresentMode::eMailbox);
		auto graphicsQueue = device->createGraphicsQueue(*swapchain);
		auto passOneTarget = device->createTexture2D(800, 600, Format::eR8G8B8A8Unorm);
		auto uniformBuffer = device->createUniformBuffer(sizeof(vec3));
		auto sampler = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirrorClampToEdge);
		
		// PASS 1
		auto colVert = parser.compileVertexShader(readFile("shader/colorVert.vert"), "main");
		auto colFrag = parser.compileFragmentShader(readFile("shader/colorFrag.frag"), "main");
		auto colProgram = device->createShaderProgram(*colVert, *colFrag);
		auto colPass = device->createRenderPass(*colProgram, 800, 600, Format::eR8G8B8A8Unorm);
		
		// PASS 2
		auto stupidVert = parser.compileVertexShader(readFile("shader/stupidVert.vert"), "main");
		auto stupidFrag = parser.compileFragmentShader(readFile("shader/stupidFrag.frag"), "main");
		auto stupidProgram = device->createShaderProgram(*stupidVert, *stupidFrag);
		auto stupidPass = device->createRenderPass(*stupidProgram, swapchain->getWidth(), swapchain->getHeight(), swapchain->getFormat(), Format::eD32SfloatS8Uint);

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
				auto cmd = device->createCommandBuffer(Usage::eReset);
				cmd->record(*colPass, *passOneTarget, [&](IRecordingCommandBuffer& commandBuffer) {
					commandBuffer
						.clearColorBuffer(0.0f, 1.0f, 0.0f, 1.0f)
						.setInput(*vertexBuffer)
						.setIndexBuffer(*indexBuffer)
						.drawIndexed(6);
				});
				auto stupidCmd = device->createCommandBuffer(Usage::eReuse);

				if (!uniformBuffer->inUse()) {
					vec3 randomColor = {
						(float) std::rand() / RAND_MAX,
						(float) std::rand() / RAND_MAX,
						(float) std::rand() / RAND_MAX };
					uniformBuffer->upload(std::vector<vec3>{ randomColor });
				}

				stupidCmd->record(*stupidPass, *swapchain, graphicsQueue->getNextFrameIndex(), [&](IRecordingCommandBuffer& commandBuffer) {
					commandBuffer.clearDepthStencilBuffer(1.0f, 0);
					commandBuffer.clearColorBuffer(1.0f, 0.0f, 0.0f, 1.0f);
					commandBuffer.setUniform("val", *uniformBuffer);
					commandBuffer.setUniform("sam", *passOneTarget, *sampler);
					commandBuffer.setInput(*stupidVertexBuffer);
					commandBuffer.setIndexBuffer(*indexBuffer);
					commandBuffer.drawIndexed(6);
				});

				graphicsQueue->submitCommands(std::vector<std::reference_wrapper<ICommandBuffer>> {
					*cmd,
					*stupidCmd
				});

				graphicsQueue->present();
			}
		}
		device->waitIdle();
	}
	std::cout << "Press enter to continue...";
	std::cin.ignore();
}
