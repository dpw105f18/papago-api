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

#include "papago.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ON
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

		return {std::move(vertex_buffer), std::move(index_buffer), 36};
	}

	std::unique_ptr<IBufferResource> vertex_buffer;
	std::unique_ptr<IBufferResource> index_buffer;
	size_t index_count;

	void use(IRecordingSubCommandBuffer& rCommandBuffer) const
	{
		rCommandBuffer.setVertexBuffer(*vertex_buffer);
		rCommandBuffer.setIndexBuffer(*index_buffer);
	}
};

struct UniformData
{
	glm::mat4 model_matrix = glm::mat4(1.0f);
};

/*
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
		subCommands.push_back(std::move(device->createSubCommandBuffer()));
	}


	{
		auto dataSize = dynamicData.size();
		for (auto i = 0; i < threadCount; ++i) {
			futures.emplace_back(threadPool.enqueue([&](size_t count, size_t offset, size_t cmdIndex) {
				auto& cmd = subCommands[cmdIndex];
				cmd->record(*renderPass, [&](IRecordingSubCommandBuffer& rSubCmd) {
					rSubCmd.setVertexBuffer(*cube->vertex_buffer);
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
*/

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
#define PARSER_COMPILER_PATH "C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe"

static std::vector<char> readPixels(const std::string& localPath, int& outWidth, int& outHeight)
{
	int texChannels;
	auto pixels = stbi_load(localPath.c_str(), &outWidth, &outHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	auto result = std::vector<char>();
	result.resize(outWidth * outHeight * 4);
	memcpy(result.data(), pixels, result.size());
	stbi_image_free(pixels);

	return result;
}

struct CubeVertex
{
	glm::vec3 pos;
	glm::vec2 uv;
};

void userTest()
{
	//init
	auto windowWidth = 800;
	auto windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);
	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);

	IDevice::Features features = { true };
	IDevice::Extensions extensions = { true, false };

	auto devices = IDevice::enumerateDevices(*surface, features, extensions, true);
	auto& device = devices[0];

	auto swapChain = device->createSwapChain(Format::eB8G8R8A8Unorm, Format::eD32Sfloat, 3, IDevice::PresentMode::eMailbox);

	std::vector<CubeVertex> cubeVertices{
		{ { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f } },
		{ { 0.5f,   0.5f,  0.5f },{ 1.0f, 1.0f } },
		{ { 0.5f,  -0.5f,  0.5f },{ 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
		{ { 0.5f,   0.5f, -0.5f },{ 1.0f, 1.0f } },
		{ { 0.5f,  -0.5f, -0.5f },{ 1.0f, 0.0f } }
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

	auto vertexShader = p.compileVertexShader(readFile("shaders/mvpTexShader.vert"), "main");
	auto fragmentShader = p.compileFragmentShader(readFile("shaders/mvpTexShader.frag"), "main");

	int texW, texH;
	auto pixels = readPixels("textures/BYcheckers.png", texW, texH);

	auto texture = device->createTexture2D(texW, texH, Format::eR8G8B8A8Unorm);
	texture->upload(pixels);

	pixels = readPixels("textures/eldorado.jpg", texW, texH);
	auto eldoTexture = device->createTexture2D(texW, texH, Format::eR8G8B8A8Unorm);
	eldoTexture->upload(pixels);

	auto sampler = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eRepeat, TextureWrapMode::eRepeat);

	auto program = device->createShaderProgram(*vertexShader, *fragmentShader);

	auto vertexBuffer = device->createVertexBuffer(cubeVertices);
	auto indexBuffer = device->createIndexBuffer(cubeIndices);

	auto renderPass = device->createRenderPass(*program, windowWidth, windowHeight, Format::eB8G8R8A8Unorm, Format::eD32Sfloat);

	auto graphicsQueue = device->createGraphicsQueue();

	std::vector<std::unique_ptr<ISubCommandBuffer>> subCommandBuffers(4);
	std::vector<std::reference_wrapper<ISubCommandBuffer>> subCommandBufferReferences;

	for (int i = 0; i < 4; ++i) {
		subCommandBuffers[i] = device->createSubCommandBuffer();
		subCommandBufferReferences.push_back(*subCommandBuffers[i]);
	}

	auto commandBuffer = device->createCommandBuffer();

	auto viewUniformBuffer = device->createUniformBuffer(sizeof(glm::mat4));
	auto instanceUniformBuffer = device->createDynamicUniformBuffer(sizeof(glm::mat4), 1000);

	std::vector<ParameterBinding> bindings;
	bindings.reserve(3);
	bindings.emplace_back( "view_projection_matrix", viewUniformBuffer.get());
	bindings.emplace_back( "model_matrix", instanceUniformBuffer.get());
	bindings.emplace_back( "sam", eldoTexture.get(), sampler.get());
	auto paramBlock = device->createParameterBlock(*renderPass, bindings);

	bindings.clear();
	bindings.emplace_back("view_projection_matrix", viewUniformBuffer.get());
	bindings.emplace_back("model_matrix", instanceUniformBuffer.get());
	bindings.emplace_back("sam", texture.get(), sampler.get());
	auto paramBlock2 = device->createParameterBlock(*renderPass, bindings);


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

			viewUniformBuffer->upload<glm::mat4>({ viewProj });
			instanceUniformBuffer->upload<glm::mat4>(world);

			ThreadPool tp(4);
			for (int t = 0; t < 4; ++t) {
				tp.enqueue([&](int t) {
					subCommandBuffers[t]->record(*renderPass, [&](IRecordingSubCommandBuffer& cmdBuf) {
						auto& block = (t % 2 == 0) ? paramBlock : paramBlock2;
						cmdBuf.setParameterBlock(*block);
						cmdBuf.setVertexBuffer(*vertexBuffer);
						cmdBuf.setIndexBuffer(*indexBuffer);

						for (int i = t * 250; i < t * 250 + 250; ++i) {
							cmdBuf.setDynamicIndex(*block, "model_matrix", i);
							cmdBuf.drawIndexed(36);
						}
					});
				}, t).wait();
			}

			commandBuffer->record(*renderPass, *swapChain, [&](IRecordingCommandBuffer& cmdBuf) {
				cmdBuf.clearColorBuffer(100U, 0U, 100U, 100U);
				cmdBuf.clearDepthBuffer(1.0f);

				cmdBuf.execute(subCommandBufferReferences);
			});

			graphicsQueue->submitCommands({ *commandBuffer });
			graphicsQueue->present(*swapChain);

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
	device->waitIdle();
}

bool handleWindowMessages(bool& run)
{
	MSG msg;
	bool pm;
	if (pm = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT) {
			run = false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return pm;
}

void triangleTest() {
	auto windowWidth = 800;
	auto windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);
	IDevice::Features features;
	features.samplerAnisotropy = true;
	IDevice::Extensions extensions;
	extensions.swapchain = true;
	extensions.samplerMirrorClampToEdge = true;


	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);

	auto parser = Parser(PARSER_COMPILER_PATH);
	auto vertexShader = parser.compileVertexShader(readFile("shaders/colorVert.vert"), "main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shaders/colorFrag.frag"), "main");

	auto device = std::move(IDevice::enumerateDevices(*surface, features, extensions)[0]);

	auto shaderProgam = device->createShaderProgram(*vertexShader, *fragmentShader);
	auto swapChain = device->createSwapChain(Format::eR8G8B8A8Unorm, 3, IDevice::PresentMode::eMailbox);
	auto renderPass = device->createRenderPass(*shaderProgam, surface->getWidth(), surface->getHeight(), swapChain->getFormat());


	std::vector<glm::vec3> vertices = { {0.0, -1.0, 0.5}, {-1.0, 0.0, 0.5}, {1.0, 0.0, 0.5} };
	auto vertexBuffer = device->createVertexBuffer(vertices);

	auto cmdBuf =  device->createCommandBuffer();
	auto subCmdBuf = device->createSubCommandBuffer();
	auto queue = device->createGraphicsQueue();

	subCmdBuf->record(*renderPass, [&](IRecordingSubCommandBuffer& subRec) {
		subRec.setVertexBuffer(*vertexBuffer);
		subRec.draw(3);
	});



	//Main game loop:
	using Clock = std::chrono::high_resolution_clock;
	auto startTime = Clock::now();
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;
	bool run = true;

	std::vector<std::reference_wrapper<ISubCommandBuffer>> subCmds;
	subCmds.emplace_back(*subCmdBuf);

	while (run)
	{
		if(handleWindowMessages(run))
		{
			//messages has been handled
		}
		else {
			auto d = "bug";
			cmdBuf->record(*renderPass, *swapChain, [&](IRecordingCommandBuffer& recCmd) {
				recCmd.clearColorBuffer(1.0f, 0.0f, 0.0f, 1.0f);
				recCmd.execute(subCmds);
			});

			queue->submitCommands({ *cmdBuf });
			queue->present(*swapChain);

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

//******************************* PHONG ***************************************

struct LightData {
	glm::vec3 pos;
	glm::vec3 viewPos;
	glm::vec3 color;
	float shinyness;
};

#include "circle_mesh.hpp"

void phongExample()
{
	auto circleMesh = circle_mesh::generateCircleMesh<6>();

	auto hwnd = StartWindow(800, 600);
	auto surface = ISurface::createWin32Surface(800, 600, hwnd);
	bool swapchainExt = true;
	auto devices = IDevice::enumerateDevices(*surface, { true }, { swapchainExt, false });
	auto& device = devices[0];
	auto swapchain = device->createSwapChain(Format::eB8G8R8A8Unorm, Format::eD32Sfloat, 3, IDevice::PresentMode::eMailbox);
	auto graphicsQueue = device->createGraphicsQueue();

	auto parser = Parser(PARSER_COMPILER_PATH);
	auto vertexShader = parser.compileVertexShader(readFile("shaders/phong.vert"), "main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shaders/phong.frag"), "main");
	auto shaderProgram = device->createShaderProgram(*vertexShader, *fragmentShader);

	auto renderpass = device->createRenderPass(*shaderProgram, 800, 600, Format::eB8G8R8A8Unorm, Format::eD32Sfloat);

	auto vertexBuffer = device->createVertexBuffer(circleMesh.vertices);
	auto indexBuffer = device->createIndexBuffer(circleMesh.indices);

	int texW, texH;
	auto texturePixels = readPixels("textures/BYcheckers.png", texW, texH);
	auto texture = device->createTexture2D(texW, texH, Format::eR8G8B8A8Unorm);
	texture->upload(texturePixels);

	auto sampler = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirroredRepeat);

	glm::mat4 translateMat = glm::translate(glm::vec3{ 0.0f, 0.0f, -3.0f });
	glm::mat4 rotateMat = glm::mat4(1.0f);

	glm::vec3 camPos = { 0.0f, 0.0f, 2.0f };
	glm::mat4 viewMat = glm::mat4(1.0f) * glm::lookAt(
		camPos,
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f }
	);

	glm::mat4 projectionMat = glm::mat4(1.0f) * glm::perspective(glm::radians(45.0f), 800.0f / 600, 1.0f, 500.0f);

	auto model = device->createUniformBuffer(sizeof(glm::mat4));
	model->upload<glm::mat4>({ translateMat * rotateMat });

	auto viewProj = device->createUniformBuffer(sizeof(glm::mat4));
	viewProj->upload<glm::mat4>({ projectionMat * viewMat * translateMat });

	glm::vec3 lightPosData = camPos - glm::vec3(0.0f, 1.0f, -7.0f);
	lightPosData = glm::vec3(0.1f, -0.4f, -9.0f); //<-- debug

	LightData lightData = {};
	lightData.pos = lightPosData;
	lightData.viewPos = camPos;
	lightData.color = { 0.2f, 0.3f, 0.7f };
	lightData.shinyness = 32.0f;

	auto lightPos = device->createUniformBuffer(sizeof(glm::vec3));
	lightPos->upload<glm::vec3>({ lightData.pos });

	auto viewPos = device->createUniformBuffer(sizeof(glm::vec3));
	viewPos->upload<glm::vec3>({ lightData.viewPos });

	auto lightColor = device->createUniformBuffer(sizeof(glm::vec3));
	lightColor->upload<glm::vec3>({ lightData.color });

	auto lightShinyness = device->createUniformBuffer(sizeof(float));
	lightShinyness->upload<float>({ lightData.shinyness });


	std::vector<ParameterBinding> bindings;
	bindings.emplace_back("model", model.get());
	bindings.emplace_back("viewProj", viewProj.get());
	bindings.emplace_back("tex", texture.get(), sampler.get());
	bindings.emplace_back("lightPos", lightPos.get());
	bindings.emplace_back("viewPos", viewPos.get());
	bindings.emplace_back("lightColor", lightColor.get());
	bindings.emplace_back("lightShinyness", lightShinyness.get());

	auto parameterBlock = device->createParameterBlock(*renderpass, bindings);

	auto cmdBuf = device->createCommandBuffer();
	auto subCmd = device->createSubCommandBuffer();

	subCmd->record(*renderpass, [&](IRecordingSubCommandBuffer& rcmd) {
		rcmd.setVertexBuffer(*vertexBuffer);
		rcmd.setIndexBuffer(*indexBuffer);
		rcmd.setParameterBlock(*parameterBlock);
		rcmd.drawIndexed(circleMesh.indices.size());
	});

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
			else if (msg.message == WM_KEYUP) {

				//'p' key
				if (msg.wParam == 80) {
					auto currentLightPos = lightPos->download<glm::vec3>()[0];
					std::cout << "light: (" << currentLightPos.x << ", " << currentLightPos.y << ", " << currentLightPos.z << ")" << std::endl;
				}
				//'r' key
				else if (msg.wParam == 'R') {

					rotateMat *= glm::rotate(glm::radians(30.00f), glm::vec3(0.3f, 1.0f, 0.0f));

				}
				else if (msg.wParam == 'I') {
					lightData.shinyness += 2;
					std::cout << "Shinyness: " << lightData.shinyness << std::endl;
				}
				else if (msg.wParam == 'J') {
					lightData.shinyness -= 2;
					std::cout << "Shinyness: " << lightData.shinyness << std::endl;
				}
				else {
					std::cout << "key code: " << std::to_string(msg.wParam) << std::endl;
				}
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {

			//FPS counter:
			auto deltaTime = (Clock::now() - lastUpdate);
			auto frameTime = (Clock::now() - lastFrame);
			lastFrame = Clock::now();


			model->upload<glm::mat4>({ translateMat * rotateMat });


			auto rotateAmount = 0.000000001f * (Clock::now() - startTime).count();
			glm::vec3 newLightPos = glm::vec3(glm::rotate(rotateAmount, glm::vec3{ 0.0f, 1.0f, 0.0f }) * glm::vec4(lightPosData, 1.0f));
			lightPos->upload<glm::vec3>({ newLightPos });

			lightShinyness->upload<float>({ lightData.shinyness });


			cmdBuf->record(*renderpass, *swapchain, [&](IRecordingCommandBuffer& recCmd) {
				recCmd.clearColorBuffer(1.0f, 0.0f, 1.0f, 1.0f);
				recCmd.clearDepthBuffer(1.0f);
				recCmd.execute({ *subCmd });
			});

			graphicsQueue->submitCommands({ *cmdBuf });
			graphicsQueue->present(*swapchain);



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

			++fps;
		}
	}

}

//*****************************************************************************

void shadowMapExample()
{
	auto hwnd = StartWindow(800, 600);
	auto surface = ISurface::createWin32Surface(800, 600, hwnd);
	auto devices = IDevice::enumerateDevices(*surface, { true }, { true, false });
	auto& device = devices[0];

	auto cube = Mesh::Cube(*device);


	auto viewMatCam = glm::lookAt(
		glm::vec3{ 0.0f, 0.0f, 10.0f },
		glm::vec3{ 0.0f, 0.0f, 0.0f },
		glm::vec3{ 0.0f, 1.0f, 0.0f }
	);

	glm::vec3 lightPos = {0.0f, -4.0f, 2.0f};
	auto viewMatLight = glm::lookAt(
		lightPos,
		glm::vec3{0.0f, 0.0f, 0.0f},
		glm::vec3{0.0f, 1.0f, 0.0f}
	);

	auto projMat = glm::perspective(
		glm::radians(90.0f),
		float(surface->getWidth()) / surface->getHeight(),
		0.1f,
		20.0f);

	auto vpMatCam = projMat * viewMatCam;
	auto vpMatLight = projMat * viewMatLight;

	auto modelMatGround = glm::translate(glm::vec3{ 0.0f, 4.0f, 0.0f }) * glm::scale(glm::vec3{ 25.0f, 1.0f, 25.0f });
	auto modelMatCube = glm::translate(glm::vec3{0.0f, 0.0f, 0.0f}) * glm::rotate(glm::radians(45.0f), glm::vec3{ 0.5f, 1.0f, 0.0f });

	auto vpDynUniform = device->createDynamicUniformBuffer(sizeof(glm::mat4), 2);
	vpDynUniform->upload<glm::mat4>({ vpMatCam, vpMatLight });

	auto modelDynUniform = device->createDynamicUniformBuffer(sizeof(glm::mat4), 2);
	modelDynUniform->upload<glm::mat4>({ modelMatGround, modelMatCube });

	auto parser = Parser(PARSER_COMPILER_PATH);

	auto colTarget = device->createTexture2D(surface->getWidth(), surface->getHeight(), Format::eR8G8B8A8Unorm);
	auto colTargetDepth = device->createDepthTexture2D(surface->getWidth(), surface->getHeight(), Format::eD32Sfloat);
	auto colSampler = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eRepeat, TextureWrapMode::eRepeat);

	int texW, texH;
	auto pixelData = readPixels("textures/white.png", texW, texH);
	auto tex = device->createTexture2D(texW, texH, Format::eR8G8B8A8Unorm);
	tex->upload(pixelData);

	auto colorVert = parser.compileVertexShader(readFile("shaders/clipCoord.vert"), "main");
	auto colorFrag = parser.compileFragmentShader(readFile("shaders/clipCoord.frag"), "main");
	auto colorProgram = device->createShaderProgram(*colorVert, *colorFrag);
	auto colorPass = device->createRenderPass(*colorProgram, colTarget->getWidth(), colTarget->getHeight(), colTarget->getFormat(), colTargetDepth->getFormat());

	std::vector<ParameterBinding> colorParams;
	colorParams.emplace_back("vp", vpDynUniform.get());
	colorParams.emplace_back("m", modelDynUniform.get());

	auto colorParamBlock = device->createParameterBlock(*colorPass, colorParams);

	auto colorCmd = device->createCommandBuffer();

	auto colorSubCmd = device->createSubCommandBuffer();
	colorSubCmd->record(*colorPass, [&](IRecordingSubCommandBuffer& rcmd) {
		rcmd.setParameterBlock(*colorParamBlock);
		rcmd.setVertexBuffer(*cube.vertex_buffer);
		rcmd.setIndexBuffer(*cube.index_buffer);
		rcmd.setDynamicIndex(*colorParamBlock, "vp", 1);	//<-- use light view

		//ground:
		rcmd.setDynamicIndex(*colorParamBlock, "m", 0);
		rcmd.drawIndexed(cube.index_count);

		//cube:
		rcmd.setDynamicIndex(*colorParamBlock, "m", 1);
		rcmd.drawIndexed(cube.index_count);
	});


	auto swapchain = device->createSwapChain(Format::eB8G8R8A8Unorm, Format::eD32Sfloat, 3, IDevice::PresentMode::eMailbox);
	auto shadowVert = parser.compileVertexShader(readFile("shaders/shadow.vert"), "main");
	auto shadowFrag = parser.compileFragmentShader(readFile("shaders/shadow.frag"), "main");
	auto shadowProgram = device->createShaderProgram(*shadowVert, *shadowFrag);
	auto shadowPass = device->createRenderPass(*shadowProgram, swapchain->getWidth(), swapchain->getHeight(), swapchain->getFormat(), Format::eD32Sfloat);

	std::vector<ParameterBinding> shadowParams;
	shadowParams.emplace_back("view_projection", vpDynUniform.get());
	shadowParams.emplace_back("shadow_view_projection", vpDynUniform.get());
	shadowParams.emplace_back("model", modelDynUniform.get());
	shadowParams.emplace_back("tex", tex.get(), colSampler.get());
	shadowParams.emplace_back("shadow_map", colTargetDepth.get(), colSampler.get());

	auto shadowParamBlock = device->createParameterBlock(*shadowPass, shadowParams);

	auto shadowCmd = device->createCommandBuffer();

	auto shadowSubCmd = device->createSubCommandBuffer();
	shadowSubCmd->record(*shadowPass, [&](IRecordingSubCommandBuffer& rcmd) {
		rcmd.setParameterBlock(*shadowParamBlock);
		rcmd.setVertexBuffer(*cube.vertex_buffer);
		rcmd.setIndexBuffer(*cube.index_buffer);
		rcmd.setDynamicIndex(*shadowParamBlock, "view_projection", 0);	//<-- camera view
		rcmd.setDynamicIndex(*shadowParamBlock, "shadow_view_projection", 1); //<-- light view
		
		//ground:
		rcmd.setDynamicIndex(*shadowParamBlock, "model", 0);
		rcmd.drawIndexed(cube.index_count);

		//cube:
		rcmd.setDynamicIndex(*shadowParamBlock, "model", 1);
		rcmd.drawIndexed(cube.index_count);
	});

	auto graphicsQueue = device->createGraphicsQueue();

	//****************************************************************************************
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
			else if (msg.message == WM_KEYUP) {
				//handle key events:
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

			lightPos = glm::vec3(glm::rotate(0.01f, glm::vec3{0.0f, 0.0f, 1.0f}) * glm::vec4(lightPos, 1.0f));
			viewMatLight = glm::lookAt(
				lightPos,
				glm::vec3{ 0.0f, 0.0f, 0.0f },
				glm::vec3{ 0.0f, 1.0f, 0.0f }
			);
			vpDynUniform->upload<glm::mat4>({ projMat * viewMatLight }, 1);

			colorCmd->record(*colorPass, *colTarget, *colTargetDepth, [&](IRecordingCommandBuffer& rcmd) {
				rcmd.clearColorBuffer(1.0f, 0.0f, 1.0f, 1.0f);
				rcmd.clearDepthBuffer(1.0f);
				rcmd.execute({ *colorSubCmd });
			});

			shadowCmd->record(*shadowPass, *swapchain, [&](IRecordingCommandBuffer& rcmd) {
				rcmd.clearColorBuffer(0.2f, 0.2f, 0.2f, 1.0f);
				rcmd.clearDepthBuffer(1.0f);
				rcmd.execute({ *shadowSubCmd });
			});

			graphicsQueue->submitCommands({ *colorCmd, *shadowCmd });
			graphicsQueue->present(*swapchain);

			++fps;
		}
	}
}

int main()
{
	try {
		//uploadTest();
		//multithreadedTest();
		//triangleTest();
		//userTest();
		//phongExample();
		shadowMapExample();
	}
	catch (std::exception e) {
		std::cout << "ERROR: " << e.what() << std::endl;
	}
	std::cout << "Press enter to continue...";
	std::cin.ignore();
	return 0;
	}
