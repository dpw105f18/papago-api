#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinUser.h>
#include <iostream>

//FPS counting and displaying in window:
#include <chrono>
#include <sstream>

//unique_ptr and multithread stuff:
#include <memory>
#include <future>

//GLM - for glsl types in user code:
#define GLM_ENABLE_EXPERIMENTAL
#include "external/glm/glm.hpp"
#include "external/glm/gtx/transform.hpp"

//File read functions (readFile(...) and readPixels(...))
#include "util.h"

//multithreading
#include "thread_pool.h"

//API
#include "external\papago\papago.hpp"


#define PARSER_COMPILER_PATH "C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe"


static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			if (MessageBox(nullptr, "Are you sure you want to quit?", "Quit", MB_YESNO | MB_ICONQUESTION) == IDYES)
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

static HWND StartWindow(size_t width, size_t height)
{
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

static void SetWindowName(HWND hwnd, const std::string& text)
{
	SetWindowText(hwnd, text.c_str());
}


struct CubeVertex
{
	glm::vec3 pos;
	glm::vec2 uv;
};

struct Vertex
{
	glm::vec3 pos;
};


void test()
{
	//init
	auto windowWidth = 800;
	auto windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);
	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);

	IDevice::Features features = { false };
	IDevice::Extensions extensions = { true, false };

	auto devices = IDevice::enumerateDevices(*surface, features, extensions);
	auto& device = devices[0];

	auto swapChain = device->createSwapChain(Format::eR8G8B8A8Unorm, Format::eD32Sfloat, 3, IDevice::PresentMode::eMailbox);

	std::vector<CubeVertex> cubeVertices {
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
		{ { 0.5f,   0.5f,  0.5f }, { 1.0f, 1.0f } },
		{ { 0.5f,  -0.5f,  0.5f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
		{ { 0.5f,   0.5f, -0.5f }, { 1.0f, 1.0f } },
		{ { 0.5f,  -0.5f, -0.5f }, { 1.0f, 0.0f } }
	};

	std::vector<uint16_t> cubeIndices{
		// Front
		0, 1, 2,
		0, 2, 3,
		// Top
		3, 7, 4,
		3, 4, 0,
		// Right
		3, 2, 6,
		3, 6, 7,
		// Back
		7, 6, 5,
		7, 5, 4,
		// Bottom
		1, 5, 6,
		1, 6, 2,
		// Left
		4, 5, 1,
		4, 1, 0
	};

	Parser p(PARSER_COMPILER_PATH);

	auto vertexShader = p.compileVertexShader(readFile("shaders/mvpTexShadr.vert"), "main");
	auto fragmentShader = p.compileFragmentShader(readFile("shaders/mvpTexShader.frag"), "main");

	int texW, texH;
	auto pixels = readPixels("textures/BYcheckers.png", texW, texH);

	auto texture = device->createTexture2D(texW, texH, Format::eR8G8B8A8Unorm);
	texture->upload(pixels);

	auto sampler = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eRepeat, TextureWrapMode::eRepeat);
		
	auto program = device->createShaderProgram(*vertexShader, *fragmentShader);

	auto vertexBuffer = device->createVertexBuffer(cubeVertices);
	auto indexBuffer = device->createIndexBuffer(cubeIndices);
	
	auto renderPass = device->createRenderPass(*program, windowWidth, windowHeight, Format::eR8G8B8A8Unorm, Format::eD32Sfloat);
	
	auto graphicsQueue = device->createGraphicsQueue(*swapChain);

	auto subCommandBuffer = device->createSubCommandBuffer();

	auto commandBuffer = device->createCommandBuffer();

	auto viewUniformBuffer = device->createDynamicUniformBuffer(sizeof(glm::mat4), 1);
	auto instanceUniformBuffer = device->createDynamicUniformBuffer(sizeof(glm::mat4) * 1000, 1000);

	renderPass->bindResource("view_projection_matrix", *viewUniformBuffer);
	renderPass->bindResource("model_matrix", *instanceUniformBuffer);

	renderPass->bindResource("sam", *texture, *sampler);
	
	//*************************************************************************************************
	//Init code here:
	
	//*************************************************************************************************
	
	//Main game loop:
	using Clock = std::chrono::high_resolution_clock;
	auto startTime = Clock::now();
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;
	bool run = true;

	glm::mat4 viewProj;
	std::vector<glm::mat4> world(1000);

	glm::vec3 translations[1000];
	for (int i = 0; i < 1000; ++i) {
		translations[i] = glm::vec3(float(rand() % 100 - 50), float(rand() % 100 - 50), -100.0f);
	}

	while (run)
	{
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) {
				run = false;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			viewProj = glm::perspective(glm::radians(45.0f), float(windowWidth) / windowHeight, 0.5f, 500.0f);

			for (int i = 0; i < 1000; ++i) {
				world[i] = glm::translate(translations[i]) *glm::rotate(float((Clock::now() - startTime).count()) * 0.000000002f + float(i), glm::vec3(0.5f, 0.5f, 0.0f));
			}			

			viewUniformBuffer->upload<glm::mat4>({viewProj});
			instanceUniformBuffer->upload<glm::mat4>(world);

			subCommandBuffer->record(*renderPass, [&](IRecordingSubCommandBuffer& cmdBuf) {
				for (int i = 0; i < 1; ++i) {
					cmdBuf.setDynamicIndex("model_matrix", i);
					cmdBuf.setVertexBuffer(*vertexBuffer);
					cmdBuf.setIndexBuffer(*indexBuffer);
					cmdBuf.drawIndexed(36);
				}
			});

			commandBuffer->record(*renderPass, *swapChain, [&](IRecordingCommandBuffer& cmdBuf) {
				cmdBuf.clearColorBuffer(255, 0, 255, 255);
				cmdBuf.clearDepthBuffer(1.0f);

				cmdBuf.execute({ *subCommandBuffer });
			});

			graphicsQueue->submitCommands({ *commandBuffer });
			graphicsQueue->present();

			//FPS counter:
			auto deltaTime = (Clock::now() - lastUpdate);
			auto frameTime = (Clock::now() - lastFrame);
			lastFrame = Clock::now();

			using namespace std::chrono_literals;
			if (deltaTime > 1s) {
				lastUpdate = Clock::now();
				std::stringstream ss;
				ss << "FPS: " << fps
					<< " --- Avg. Frame Time: " << 1000.0 / fps << "ms"
					<< " --- Last Frame Time: " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(frameTime).count() << "ms";
				SetWindowName(hwnd, ss.str());
				fps = 0;
			}

			//*************************************************************************************************
			//Loop code here:

			//*************************************************************************************************
			++fps;
		}
	}
}

int main()
{	
	test();
}