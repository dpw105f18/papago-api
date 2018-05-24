
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinUser.h>
#include <iostream>
#include <chrono>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#include "external\glm\glm.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "external\glm\gtx\transform.hpp"

#include "../papago-api-core/include/papago.hpp"
#include "thread_pool.h"
#include "test_config.hpp"
#include "wmi_accessor.hpp"
#include "Camera.h"
#include "RenderObject.h"
#include "Scene.h"


LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

HWND StartWindow(size_t width, size_t height)
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

void SetWindowName(HWND hwnd, const std::string& text)
{
	SetWindowTextA(hwnd, text.c_str());
}

std::string readFile(const std::string& file_path) {
	std::ifstream stream(file_path, std::ios::ate);
	auto size = stream.tellg();
	stream.seekg(0);
	std::string result = std::string(size, '\0');
	stream.read(&result[0], size);
	return result;
}

//Returns pixel data in a std::vector<char>. Pixels are loaded in a RGBA format.
//the height and width of the image is provided through the parameters outWidth and outHeight.
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

void SaveToFile(const std::string& file, const std::string& data)
{
	std::ofstream fs;
	fs.open(file, std::ofstream::app);
	fs << data;
	fs.close();
}

struct CubeVertex
{
	glm::vec3 pos;
	glm::vec2 uv;
};

void main(int argc, char* argv[])
{
	std::stringstream arg;

	for (auto i = 0; i < argc; ++i) {
		arg << argv[i] << " ";
	}
	TestConfiguration::SetTestConfiguration(arg.str().c_str());
	auto testConfig = TestConfiguration::GetInstance();
	ThreadPool threadPool{ testConfig.drawThreadCount };

	auto windowWidth = 800;
	auto windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);

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


	//Load scene:
	auto cubeCountPerDim = testConfig.cubeDimension;
	auto paddingFactor = testConfig.cubePadding;

	Camera camera = Camera::Default();
	auto heightFOV = camera.FieldOfView() / (float(windowWidth) / float(windowHeight));
	auto base = (cubeCountPerDim + (cubeCountPerDim - 1) * paddingFactor) / 2.0f;
	auto camDistance = base / std::tan(heightFOV / 2);
	float z = camDistance + base + camera.Near();

	camera.SetPosition({ 0.0f, 0.0f, z, 1.0f });
	auto magicFactor = 2;
	camera.SetFar(magicFactor * (z + base + camera.Near()));
	auto scene = Scene(camera, cubeCountPerDim, paddingFactor);

	//init PapaGo:

	auto& surface = ISurface::createWin32Surface( 800, 600, hwnd);
	auto anisotropicFeature = true;
	auto swapchainExt = true;
	auto mirrorClapToEdgeExt = false;

	auto devices = IDevice::enumerateDevices(*surface, { anisotropicFeature }, { swapchainExt, mirrorClapToEdgeExt });
	auto& device = devices[0];

	auto& swapchain = device->createSwapChain(Format::eR8G8B8A8Unorm, Format::eD32Sfloat, 3, IDevice::PresentMode::eMailbox);
	auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe");
	
	auto vertexBuffer = device->createVertexBuffer(cubeVertices);
	auto indexBuffer = device->createIndexBuffer(cubeIndices);

	//TODO: enable skulls
	auto vertexShader = parser.compileVertexShader(readFile("shaders/shader.vert"), "main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shaders/shader.frag"), "main");
	auto shaderProgram = device->createShaderProgram(*vertexShader, *fragmentShader);

	auto& renderpass = device->createRenderPass(*shaderProgram, surface->getWidth(), surface->getHeight(), swapchain->getFormat(), Format::eD32Sfloat);

	auto commandBuffer = device->createCommandBuffer();

	std::vector<std::unique_ptr<ISubCommandBuffer>> subCmds;
	subCmds.reserve(testConfig.drawThreadCount);
	for (auto i = 0; i < testConfig.drawThreadCount; ++i) {
		subCmds.emplace_back(device->createSubCommandBuffer());
	}
	
	auto subCmdRefs = std::vector<std::reference_wrapper<ISubCommandBuffer>>();
	for (auto& scmd : subCmds) {
		subCmdRefs.emplace_back(*scmd);
	}
	
	
	
	
	int texW, texH;
	auto texPixels = readPixels("textures/texture.png", texW, texH);
	auto texture = device->createTexture2D(texW, texH, Format::eR8G8B8A8Unorm);
	texture->upload(texPixels);

	auto sampler = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eRepeat, TextureWrapMode::eRepeat);

	auto projection = device->createUniformBuffer(sizeof(glm::mat4));
	auto view = device->createUniformBuffer(sizeof(glm::mat4));
	auto model = device->createDynamicUniformBuffer(sizeof(glm::mat4), scene.renderObjects().size());

	std::vector<ParameterBinding> bindings
	{
		{"projection", projection.get()},
		{"view", view.get()},
		{"model", model.get()},
		{"texSampler", texture.get(), sampler.get()}
	};

	auto parameterBlock = device->createParameterBlock(*renderpass, bindings);

	auto graphicsQueue = device->createGraphicsQueue();

	//*********************************************************
	//Main game loop:
	using Clock = std::chrono::high_resolution_clock;
	auto startTime = Clock::now();
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;
	long oldFps = 0;
	bool run = true;

	std::stringstream frametimeCsv;
	frametimeCsv << "frametime(nanoseconds)\n";
	
	std::stringstream fpsCsv;
	fpsCsv << "FPS\n";

	int probeCount = 0;
	
	WMIAccessor wmiAccessor;
	_bstr_t probeProperties[] = { "Identifier", "Value", "SensorType" };

	if (testConfig.openHardwareMonitorData) {
		wmiAccessor.Connect("OpenHardwareMonitor");
	}
	DataCollection<WMIDataItem> wmiCollection;

	auto runTime = Clock::now() - startTime;

	while (run && 
		(testConfig.seconds == 0 ||
		std::chrono::duration<double, std::milli>(runTime).count() < testConfig.seconds * 1000
		))
	{
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) {
				run = false;
			}
			else if (msg.message == WM_KEYDOWN) {
				//if key pressed is "r":
				if (msg.wParam == 82) {
					auto& rc = TestConfiguration::GetInstance().rotateCubes;
					rc = !rc;
				}
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			auto deltaTime = (Clock::now() - lastUpdate);
			auto frameTime = (Clock::now() - lastFrame);
			lastFrame = Clock::now();

			auto deltaTimeMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(deltaTime).count();

			using namespace std::chrono_literals;
			if (deltaTime > 1s) {
				lastUpdate = Clock::now();
				std::stringstream ss;
				ss << "FPS: " << fps
					<< " --- Avg. Frame Time: " << deltaTimeMs / fps << "ms"
					<< " --- Last Frame Time: " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(frameTime).count() << "ms";
				SetWindowName(hwnd, ss.str());
				
				if (TestConfiguration::GetInstance().recordFPS) {
					fpsCsv << oldFps << "\n";
				}

				oldFps = fps;
				fps = 0;

				if (TestConfiguration::GetInstance().recordFrameTime) {
					frametimeCsv << deltaTime.count() << "\n";
				}
			}

			//OHM
			if (testConfig.openHardwareMonitorData &&
				deltaTimeMs > probeCount * testConfig.probeInterval)
			{
				//QueryItem queries one item from WMI, which is separated into multiple items (put in a vector here)
				auto items = wmiAccessor.QueryItem("sensor", probeProperties, 3);

				//each item needs a timestamp and an id ( = probeCount) and the last completely measured FPS
				for (auto& item : items) {
					item.Timestamp = std::to_string(runTime.count());
					item.Id = std::to_string(probeCount);
					item.FPS = std::to_string(oldFps);
					wmiCollection.Add(item);
				}

				++probeCount;
			}

			//update uniform buffers:
			glm::mat4 newView = glm::lookAt(
				glm::vec3(camera.Position()),
				glm::vec3(camera.Target()),
				glm::vec3(camera.Up())
			);

			view->upload<glm::mat4>({ newView });

			glm::mat4 newProjection = glm::perspective(
				camera.FieldOfView(),
				float(windowWidth) / windowHeight,
				camera.Near(),
				camera.Far()
			);

			projection->upload<glm::mat4>({newProjection});
			std::vector<glm::mat4> dynamicBufferData;
			//dynamic uniform buffer:
			for (auto index = 0; index < scene.renderObjects().size(); index++)
			{
				auto& render_object = scene.renderObjects()[index];
				auto newModel = glm::mat4(1.0f);
				newModel = translate(glm::mat4(1.0f), { render_object.x(), render_object.y(), render_object.z() });

				//hack around the const to update m_RotationAngle. //TODO: remove rotation feature or const from m_Scene.renderObjects()
				auto noconst = const_cast<RenderObject*>(&render_object);
				noconst->m_RotationAngle = (render_object.m_RotationAngle + 1) % 360;

				if (TestConfiguration::GetInstance().rotateCubes) {
					auto rotateX = 0.0001f*(index + 1) * std::pow(-1, index);
					auto rotateY = 0.0002f*(index + 1) * std::pow(-1, index);
					auto rotateZ = 0.0003f*(index + 1) * std::pow(-1, index);
					newModel = glm::rotate<float>(newModel, render_object.m_RotationAngle * 3.14159268 / 180, glm::tvec3<float>{ rotateX, rotateY, rotateZ });
				}

				dynamicBufferData.push_back(newModel);
			}

			model->upload(dynamicBufferData);

			//record commands
			std::vector<std::future<void>> futures;
			
			for (auto i = 0; i < testConfig.drawThreadCount; ++i) {
				auto scmd = subCmdRefs[i];
				futures.emplace_back(
					threadPool.enqueue([&](std::reference_wrapper<ISubCommandBuffer> cmd, int threadIndex, const std::vector<RenderObject>& renderObjects) {
						cmd.get().record(*renderpass, [&](IRecordingSubCommandBuffer& rcmd) {
							rcmd.setVertexBuffer(*vertexBuffer);
							rcmd.setIndexBuffer(*indexBuffer);
							rcmd.setParameterBlock(*parameterBlock);
							
							auto threadCount = testConfig.drawThreadCount;
							auto roCount = renderObjects.size() / threadCount;
							auto stride = roCount;

							//last thread handles the remainder after integer division
							if (threadIndex == threadCount - 1) {
								roCount += renderObjects.size() % threadCount;
							}
							
							for (auto j = 0; j < roCount; ++j) {
								rcmd.setDynamicIndex(*parameterBlock, "model", threadIndex * stride + j);
								rcmd.drawIndexed(cubeIndices.size());
							}
						});
					},scmd, i, scene.renderObjects())
				);
			}
			
			//draw frame:
			for (auto& f : futures) {
				f.wait();
			}

			commandBuffer->record(*renderpass, *swapchain, [&](IRecordingCommandBuffer& rcmd) {
				rcmd.clearColorBuffer(0.0f, 0.0f, 0.0f, 1.0f);
				rcmd.clearDepthBuffer(1.0f);
				rcmd.execute(subCmdRefs);
			});

			
			graphicsQueue->submitCommands({ *commandBuffer });
			graphicsQueue->present(*swapchain);

			++fps;
		}

		runTime = Clock::now() - startTime;
	}

	device->waitIdle();

	//save files
	auto now = time(NULL);
	tm* localNow = new tm();
	localtime_s(localNow, &now);

	auto yearStr = std::to_string((1900 + localNow->tm_year));
	auto monthStr = localNow->tm_mon < 9 ? "0" + std::to_string(localNow->tm_mon + 1) : std::to_string(localNow->tm_mon + 1);
	auto dayStr = localNow->tm_mday < 10 ? "0" + std::to_string(localNow->tm_mday) : std::to_string(localNow->tm_mday);
	auto hourStr = localNow->tm_hour < 10 ? "0" + std::to_string(localNow->tm_hour) : std::to_string(localNow->tm_hour);
	auto minStr = localNow->tm_min < 10 ? "0" + std::to_string(localNow->tm_min) : std::to_string(localNow->tm_min);
	auto secStr = localNow->tm_sec < 10 ? "0" + std::to_string(localNow->tm_sec) : std::to_string(localNow->tm_sec);

	auto fname = yearStr + monthStr + dayStr + hourStr + minStr + secStr;

	if (testConfig.exportCsv && testConfig.openHardwareMonitorData) {

		auto csvStr = wmiCollection.MakeString(";");
		SaveToFile("data_" + fname + ".csv", csvStr);
	}

	if (testConfig.exportCsv) {
		auto csvStr = testConfig.MakeString(";");
		SaveToFile("conf_" + fname + ".csv", csvStr);
	}

	if (testConfig.recordFPS) {
		SaveToFile("fps_" + fname + ".csv", fpsCsv.str());
	}

	if (testConfig.recordFrameTime) {
		SaveToFile("frameTime_" + fname + ".csv", frametimeCsv.str());
	}

	delete localNow;
};