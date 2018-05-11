#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinUser.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>

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

#define GLM_ENABLE_EXPERIMENTAL
#include "external/glm/glm.hpp"
#include "external/glm/gtx/transform.hpp"

#include "thread_pool.h"


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

void SetWindowName(HWND hwnd, const std::string& text)
{
	SetWindowTextA(hwnd, text.c_str());
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


struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
};

struct Mesh
{
	static Mesh Cube(IDevice& device)
	{
		auto vertex_buffer = device.createVertexBuffer(std::vector<Vertex>{
			{ { -0.5f, -0.5f, 0.5f }, {0.0f, 0.0f}},
			{{ -0.5f,  0.5f,  0.5f }, {0.0f, 1.0f}},
			{{  0.5f,  0.5f,  0.5f }, {1.0f, 1.0f}},
			{{  0.5f, -0.5f,  0.5f }, {1.0f, 0.0f}},
			{{ -0.5f, -0.5f, -0.5f }, {0.0f, 0.0f}},
			{{ -0.5f,  0.5f, -0.5f }, {0.0f, 1.0f}},
			{{  0.5f,  0.5f, -0.5f }, {1.0f, 1.0f}},
			{{  0.5f, -0.5f, -0.5f }, {1.0f, 0.0f}}
		});

		auto index_buffer = device.createIndexBuffer(std::vector<uint16_t>{
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
		});

		return {std::move(vertex_buffer), std::move(index_buffer), 48};
	}

	std::unique_ptr<IBufferResource> vertex_buffer;
	std::unique_ptr<IBufferResource> index_buffer;
	size_t index_count;

	void use(IRecordingCommandBuffer& rCommandBuffer) const
	{
		rCommandBuffer.setInput(*vertex_buffer);
		rCommandBuffer.setIndexBuffer(*index_buffer);
	}
};

struct UniformData
{
	glm::mat4 model_matrix = glm::mat4(1.0f);
};

void multithreadedTest() {
	const size_t windowWidth = 800;
	const size_t windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);
	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);
	IDevice::Features features{ };
	features.samplerAnisotropy = true;
	IDevice::Extensions extensions{ };
	extensions.swapchain = true;
	auto devices = IDevice::enumerateDevices(*surface, features, extensions);
	auto& device = devices[0];


	auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe");
	auto swapchain = device->createSwapChain(Format::eR8G8B8A8Unorm, 3, IDevice::PresentMode::eMailbox);
	auto graphicsQueue = device->createGraphicsQueue(*swapchain);
	auto viewProjectionMatrix = device->createUniformBuffer(sizeof(glm::mat4));

	glm::vec3 grid = { 2, 2, 2 };
	glm::vec3 padding = {2.0f, 2.0f, 2.0f};
	glm::vec3 dim = 0.5f * grid;
	std::vector<UniformData> dynamicData;

	for (auto x = -dim.x; x < dim.x; ++x) {
		for (auto y = -dim.y; y < dim.y; ++y) {
			for (auto z = -dim.z; z < dim.z; ++z) {
				dynamicData.push_back({ glm::translate(glm::vec3{ x, y, z } * padding) });
			}
		}
	}
	
	auto d_buffer = device->createDynamicUniformBuffer(sizeof(UniformData), dynamicData.size());

	d_buffer->upload<UniformData>(dynamicData);

	auto dbug = d_buffer->download<glm::mat4>();

	auto cube = std::make_shared<Mesh>(Mesh::Cube(*device));
	
	{
		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, grid.z * 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 projection = glm::perspective(
			glm::radians(90.0f),
			surface->getWidth() * 1.0f / surface->getHeight(),
			1.0f,
			2500.0f);

		// TODO: make it so that vector is not mandatory
		viewProjectionMatrix->upload<glm::mat4>({
			projection * view
		});
	}

	auto vertexShader = parser.compileVertexShader(readFile("shader/mvpTexShader.vert"), "main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shader/mvpTexShader.frag"), "main");
	auto program = device->createShaderProgram(*vertexShader, *fragmentShader);
	auto renderPass = device->createRenderPass(*program, 800, 600, swapchain->getFormat());

	using Clock = std::chrono::high_resolution_clock;
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;

	size_t threadCount = 1;
	ThreadPool threadPool = { threadCount };

	int width, height, comp;
	auto pixels = stbi_load("textures/eldorado.jpg", &width, &height, &comp, 4);

	auto imageData = std::vector<char>(width * height * 4);	
	memcpy(imageData.data(), pixels, width* height * 4);
	auto image = device->createTexture2D(width, height, Format::eR8G8B8A8Unorm);
	image->upload(imageData);

	auto sam = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirroredRepeat);

	renderPass->bindResource("view_projection_matrix", *viewProjectionMatrix);
	renderPass->bindResource("model_matrix", *d_buffer);
	renderPass->bindResource("sam", *image, *sam);

	std::vector<std::future<void>> futures;
	std::vector<std::unique_ptr<ISubCommandBuffer>> subCommands;
	subCommands.reserve(threadCount);
	std::vector<std::unique_ptr<ICommandBuffer>> commandBuffers;
	commandBuffers.push_back(device->createCommandBuffer());
	auto& commandBuffer = commandBuffers[0];

	for (auto i = 0; i < threadCount; ++i) {
		subCommands.push_back(std::move(commandBuffer->createSubCommandBuffer()));
	}


	{
		auto dataSize = dynamicData.size();
		for (auto i = 0; i < threadCount; ++i) {
			futures.emplace_back(threadPool.enqueue([&](size_t count, size_t offset, size_t cmdIndex) {
				auto& cmd = subCommands[cmdIndex];
				cmd->record(*renderPass, [&](IRecordingSubCommandBuffer& rSubCmd) {
					rSubCmd.setInput(*cube->vertex_buffer);
					rSubCmd.setIndexBuffer(*cube->index_buffer);
					for (auto j = 0; j < count; ++j) {
						rSubCmd.setDynamicIndex("model_matrix", offset + j);
						rSubCmd.drawIndexed(36);
					}
				});

			}, dataSize / threadCount, (dataSize / threadCount) * i, i));

		}//end for

		for (auto& f : futures) {
			f.wait();
		}
		futures.clear();
	}

	bool keyPressed = false;
	glm::mat4 rotateMat = glm::mat4(1.0f);
	float rotateFudgeVal = 0.005f;

	while (true)
	{
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) {
				break;
			}
			else if (msg.message == WM_KEYDOWN) {
				switch (msg.wParam)
				{
				case 87:
					if (!keyPressed)
					{
						rotateMat = glm::rotate(rotateMat, rotateFudgeVal * 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
						keyPressed = true;
					}
					break;
				case 65:
					if (!keyPressed) {
						rotateMat = glm::rotate(rotateMat, rotateFudgeVal *2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
						keyPressed = true;
					}
					break;
				case 83:
					if (!keyPressed) {
						rotateMat = glm::rotate(rotateMat, rotateFudgeVal * -2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
						keyPressed = true;
					}
					break;
				case 68:
					if (!keyPressed) {
						rotateMat = glm::rotate(rotateMat, rotateFudgeVal * -2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
						keyPressed = true;
					}
					break;
				}

				if (msg.wParam == 82) {
					/*
					auto& rc = TestConfiguration::GetInstance().rotateCubes;
					rc = !rc;
					*/
				}
			}
			else if (msg.message == WM_KEYUP) {
				switch (msg.wParam) {
				case 87:
					keyPressed = false;
					break;
				case 65:
					keyPressed = false;
					break;
				case 83:
					keyPressed = false;
					break;
				case 68:
					keyPressed = false;
					break;
				}
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
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

			//Make dem cubes dance!
			if (keyPressed) {
				for (auto& modelMat : dynamicData) {
					modelMat.model_matrix *= rotateMat;
				}
				d_buffer->upload(dynamicData);
			}

			commandBuffer->record(*renderPass, *swapchain, [&](IRecordingCommandBuffer& rCommandBuffer) {
				//cube->use(rCommandBuffer);
				rCommandBuffer.clearColorBuffer(1.0f, 0.0f, 0.0f, 1.0f);
				rCommandBuffer.execute(subCommands);
			});

			std::vector<std::reference_wrapper<ICommandBuffer>> submissions;
			for(auto& commandBuffer : commandBuffers)
			{
				submissions.emplace_back(*commandBuffer);
			}

			graphicsQueue->submitCommands(submissions);

			graphicsQueue->present();
			fps++;
		}
	}
	device->waitIdle();
}

void uploadTest()
{
	auto hwnd = StartWindow(100, 100);
	auto surface = ISurface::createWin32Surface(100, 100, hwnd);
	auto devices = IDevice::enumerateDevices(*surface, {}, {});
	auto& device = devices[0];

	auto dynRes = device->createDynamicUniformBuffer(sizeof(float), 10);
	dynRes->upload<float>({ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
					1.0f, 1.0f, 1.0f, 1.0f, 1.0f });

	auto dl = dynRes->download<float>();

	dynRes->upload<float>({ 5.0f, 5.0f, 5.0f }, 4);

	dl = dynRes->download<float>();

	auto dl6 = dynRes->download<float>(6);

	auto d = "bug";
}

int main()
{
	try {
		uploadTest();
		//multithreadedTest();
	}
	catch (std::exception e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	std::cout << "Press enter to continue...";
	std::cin.ignore();
	return 0;
	/*
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
		auto swapchain = device->createSwapChain(Format::eR8G8B8A8Unorm, 3, IDevice::PresentMode::eMailbox);
		auto graphicsQueue = device->createGraphicsQueue(*swapchain);
		auto passOneTarget = device->createTexture2D(800, 600, Format::eR8G8B8A8Unorm);
		auto uniformBuffer = device->createUniformBuffer(sizeof(vec3));
		auto sampler = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirrorClampToEdge);
		
		// PASS 1
		auto colVert = parser.compileVertexShader(readFile("shader/colorVert.vert"), "main");
		auto colFrag = parser.compileFragmentShader(readFile("shader/colorFrag.frag"), "main");
		auto colProgram = device->createShaderProgram(*colVert, *colFrag);
		auto colPass = device->createRenderPass(*colProgram, 800, 600, Format::eR8G8B8A8Unorm, false);
		
		// PASS 2
		auto stupidVert = parser.compileVertexShader(readFile("shader/stupidVert.vert"), "main");
		auto stupidFrag = parser.compileFragmentShader(readFile("shader/stupidFrag.frag"), "main");
		auto stupidProgram = device->createShaderProgram(*stupidVert, *stupidFrag);
		auto stupidPass = device->createRenderPass(*stupidProgram, swapchain->getWidth(), swapchain->getHeight(), swapchain->getFormat(),  true);

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
	*/
}
