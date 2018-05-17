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


	
	//*************************************************************************************************
	//Init code here:

	auto surface = ISurface::createWin32Surface(
		windowWidth, windowHeight,
		hwnd);

	auto devices = IDevice::enumerateDevices(
		*surface,
		{} /* features */,
		{} /* extensions */);
	auto& device = devices[0]; // Pick first device

	std::vector<glm::vec3> triangleVerticies{
		{0.0, 0.0, 0.0}, {0.5, 1.0, 0.0}, {1.0, 0.0, 0.0}
	};

	std::vector<uint16_t> triangleIndicies{
		0, 1, 2
	};

	auto sampler2D = device->createTextureSampler2D(Filter::eNearest, Filter::eNearest, TextureWrapMode::eClampToEdge, TextureWrapMode::eClampToEdge);
	
	int width;
	int height;
	auto pixelData = readPixels("textures/BYcheckers.png", width, height);
	auto tex = device->createTexture2D(width, height, Format::eR8G8B8A8Unorm);
	tex->upload(pixelData);



	auto vertexBuffer = device->createVertexBuffer(cubeVertices);
	auto indexBuffer = device->createIndexBuffer(cubeIndices);



	//unique_ptr<ISurface> surface = // create Surface



	auto parser = Parser(PARSER_COMPILER_PATH);

	auto vertexShader = parser.compileVertexShader(readFile("shaders/mvpTexShader.vert"),"main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shaders/mvpTexShader.frag"), "main");

	auto shaderProgram = device->createShaderProgram(*vertexShader, *fragmentShader);
	auto renderPass = device->createRenderPass(*shaderProgram, windowWidth, windowHeight, Format::eR8G8B8A8Unorm);

	renderPass->bindResource("sam", *tex, *sampler2D);

	auto vpBuffer = device->createUniformBuffer(sizeof(glm::mat4));
	vpBuffer->upload<glm::mat4>({ glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.3f, 1000.0f) });
	auto mBuffer = device->createUniformBuffer(sizeof(glm::mat4));

	auto mBufferDyn = device->createDynamicUniformBuffer(sizeof(glm::mat4), 2);

	

	renderPass->bindResource("view_projection_matrix", *vpBuffer);
	renderPass->bindResource("model_matrix", *mBufferDyn);

	auto commandBuffer = device->createCommandBuffer();

	auto swapChain = device->createSwapChain(Format::eR8G8B8A8Unorm, 3, IDevice::PresentMode::eMailbox);

	auto subCommandBuffer = device->createSubCommandBuffer();
	subCommandBuffer->record(*renderPass, [&](IRecordingSubCommandBuffer& subRec) {
		subRec.setVertexBuffer(*vertexBuffer);
		subRec.setIndexBuffer(*indexBuffer);
		subRec.setDynamicIndex("model_matrix", 0);
		subRec.drawIndexed(cubeIndices.size());
		subRec.setDynamicIndex("model_matrix", 1);
		subRec.drawIndexed(cubeIndices.size());
	});


	auto graphicsQueue = device->createGraphicsQueue(*swapChain);


	auto cubepos = glm::translate(glm::vec3(0.0f, 0.0f, -2.0f));
	auto cubeRot = glm::rotate(glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));



	//*************************************************************************************************
	
	//Main game loop:
	using Clock = std::chrono::high_resolution_clock;
	auto startTime = Clock::now();
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;
	bool run = true;

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


			cubeRot *= glm::rotate(glm::radians(0.1f), glm::vec3(1.0f, 0.0f, 0.0f));
			mBuffer->upload<glm::mat4>({cubepos * cubeRot });
			
			mBufferDyn->upload<glm::mat4>({ cubepos * glm::translate(glm::vec3(1.0f, 0.0f, 0.0f))  * cubeRot }, 0);
			mBufferDyn->upload<glm::mat4>({ cubepos * glm::translate(glm::vec3(-1.0f, 0.0f, 0.0f))  * cubeRot }, 1);

			commandBuffer->record(*renderPass, *swapChain, [&](IRecordingCommandBuffer& recCommand) {
				recCommand.clearColorBuffer(0.0f, 0.0f, 0.0f, 1.0f);
				recCommand.execute({ *subCommandBuffer });
			});
			graphicsQueue->submitCommands({ *commandBuffer });
			graphicsQueue->present();

			//*************************************************************************************************
			++fps;
		}
	}
}

int main()
{	
	test();
}