//FPS counting and displaying in window:
#include <chrono>
#include <sstream>
#include <memory>
#include <future>

//GLM - for glsl types in user code:
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_SWIZZLE_XYZW
#include "external/glm/glm.hpp"
#include "external/glm/gtx/transform.hpp"

//File read functions (readFile(...) and readPixels(...))
#include "util.h"

//API::
#include "external/papago/papago.hpp"

//Vertex data and structure:
#include "mesh.h"

//Thread pool
#include "thread_pool.h"

//convenience:
#define PARSER_COMPILER_PATH "C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe"
using CommandBufferRefCollection = std::vector<std::reference_wrapper<ICommandBuffer>>;


void example_triangle()
{
	//init
	auto windowWidth = 800;
	auto windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);

	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);
	IDevice::Features features;
	features.samplerAnisotropy = true;

	IDevice::Extensions extensions;
	extensions.swapchain = true;
	auto devices = IDevice::enumerateDevices(*surface, features, extensions);
	auto& device = devices[0];

	//TODO: make readFile visible
	auto colorVertCode = readFile("shaders/colorVert.vert");
	auto colorFragCode = readFile("shaders/colorFrag.frag");

	auto parser = Parser(PARSER_COMPILER_PATH);

	auto vertShader = parser.compileVertexShader(colorVertCode, "main");
	auto fragShader = parser.compileFragmentShader(colorFragCode, "main");
	auto shaderProgram = device->createShaderProgram(*vertShader, *fragShader);

	auto renderPass = device->createRenderPass(*shaderProgram, windowWidth, windowHeight, Format::eB8G8R8A8Unorm);

	//TODO: make eMailBox default? there is only one..
	auto swapChain = device->createSwapChain(Format::eB8G8R8A8Unorm, 3, IDevice::PresentMode::eMailbox);

	auto triangleVertices = std::vector<Vertex>{
		{ { 0.5f, -0.5f, 0.5f } },
		{ { -0.5f, -0.5f, 0.5f } },
		{ { 0.0f,  0.5f, 0.5f } },
	};

	auto vertexBuffer = device->createVertexBuffer(triangleVertices);

	auto scmd = device->createSubCommandBuffer();

	auto scmds = std::vector<std::unique_ptr<ISubCommandBuffer>>();

	scmd->record(*renderPass, [&](IRecordingSubCommandBuffer& rscmd) {
		rscmd.setVertexBuffer(*vertexBuffer);

		//TODO: rename parameters in interface:
		rscmd.draw(3);
	});

	scmds.emplace_back(std::move(scmd));

	auto cmd = device->createCommandBuffer();

	CommandBufferRefCollection cmds = { *cmd };

	auto graphicsQueue = device->createGraphicsQueue(*swapChain);

	//Main game loop:
	using Clock = std::chrono::high_resolution_clock;
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

			//Handle input:

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

			//Handle game logic:
			cmd->record(*renderPass, *swapChain, [&](IRecordingCommandBuffer& rcmd) {
				rcmd.clearColorBuffer(0, 0, 0, 1);
				rcmd.execute(scmds);
			});

			graphicsQueue->submitCommands(cmds);
			graphicsQueue->present();

			++fps;
		}
	}
}

void example_cubes()
{
	const size_t windowWidth = 800;
	const size_t windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);
	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);
	IDevice::Features features{};
	features.samplerAnisotropy = true;
	IDevice::Extensions extensions{};
	extensions.swapchain = true;
	auto devices = IDevice::enumerateDevices(*surface, features, extensions);
	auto& device = devices[0];


	auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe");
	auto swapchain = device->createSwapChain(Format::eR8G8B8A8Unorm, 3, IDevice::PresentMode::eMailbox);
	auto graphicsQueue = device->createGraphicsQueue(*swapchain);
	auto viewProjectionMatrix = device->createUniformBuffer(sizeof(glm::mat4));

	glm::vec3 grid = { 2, 2, 2 };
	glm::vec3 padding = { 2.0f, 2.0f, 2.0f };
	glm::vec3 dim = 0.5f * grid;
	std::vector<glm::mat4> dynamicData;

	for (auto x = -dim.x; x < dim.x; ++x) {
		for (auto y = -dim.y; y < dim.y; ++y) {
			for (auto z = -dim.z; z < dim.z; ++z) {
				dynamicData.push_back({ glm::translate(glm::vec3{ x, y, z } *padding) });
			}
		}
	}

	auto d_buffer = device->createDynamicUniformBuffer(sizeof(glm::mat4), dynamicData.size());

	d_buffer->upload<glm::mat4>(dynamicData);

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

	auto vertexShader = parser.compileVertexShader(readFile("shaders/mvpTexShader.vert"), "main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shaders/mvpTexShader.frag"), "main");
	auto program = device->createShaderProgram(*vertexShader, *fragmentShader);
	auto renderPass = device->createRenderPass(*program, 800, 600, swapchain->getFormat());

	using Clock = std::chrono::high_resolution_clock;
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;

	int width, height, comp;
	auto pixels = stbi_load("textures/BYcheckers.png", &width, &height, &comp, 4);

	auto imageData = std::vector<char>(width * height * 4);
	memcpy(imageData.data(), pixels, width* height * 4);
	auto image = device->createTexture2D(width, height, Format::eR8G8B8A8Unorm);
	image->upload(imageData);

	auto sam = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirroredRepeat);

	renderPass->bindResource("view_projection_matrix", *viewProjectionMatrix);
	renderPass->bindResource("model_matrix", *d_buffer);
	renderPass->bindResource("sam", *image, *sam);

	std::vector<std::unique_ptr<ISubCommandBuffer>> subCommands;
	subCommands.push_back(std::move(device->createSubCommandBuffer()));

	std::vector<std::unique_ptr<ICommandBuffer>> commandBuffers;
	commandBuffers.push_back(device->createCommandBuffer());
	auto& commandBuffer = commandBuffers[0];

	{
		auto dataSize = dynamicData.size();
		auto& cmd = subCommands[0];
		cmd->record(*renderPass, [&](IRecordingSubCommandBuffer& rSubCmd) {
			rSubCmd.setVertexBuffer(*cube->vertex_buffer);
			rSubCmd.setIndexBuffer(*cube->index_buffer);
				rSubCmd.setDynamicIndex("model_matrix", 0);
				rSubCmd.drawIndexed(36);
				rSubCmd.setDynamicIndex("model_matrix", 1);
				rSubCmd.drawIndexed(36);
		});
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

			commandBuffer->record(*renderPass, *swapchain, [&](IRecordingCommandBuffer& rCommandBuffer) {
				//cube->use(rCommandBuffer);
				rCommandBuffer.clearColorBuffer(0.0f, 0.5f, 0.7f, 1.0f);
				rCommandBuffer.execute(subCommands);
			});

			std::vector<std::reference_wrapper<ICommandBuffer>> submissions;
			for (auto& commandBuffer : commandBuffers)
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

void example_cubes_parallel()
{
	const size_t windowWidth = 800;
	const size_t windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);
	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);
	IDevice::Features features{};
	features.samplerAnisotropy = true;
	IDevice::Extensions extensions{};
	extensions.swapchain = true;
	auto devices = IDevice::enumerateDevices(*surface, features, extensions);
	auto& device = devices[0];


	auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe");
	auto swapchain = device->createSwapChain(Format::eR8G8B8A8Unorm, 3, IDevice::PresentMode::eMailbox);
	auto graphicsQueue = device->createGraphicsQueue(*swapchain);
	auto viewProjectionMatrix = device->createUniformBuffer(sizeof(glm::mat4));

	glm::vec3 grid = { 2, 2, 2 };
	glm::vec3 padding = { 2.0f, 2.0f, 2.0f };
	glm::vec3 dim = 0.5f * grid;
	std::vector<glm::mat4> dynamicData;

	for (auto x = -dim.x; x < dim.x; ++x) {
		for (auto y = -dim.y; y < dim.y; ++y) {
			for (auto z = -dim.z; z < dim.z; ++z) {
				dynamicData.push_back({ glm::translate(glm::vec3{ x, y, z } *padding) });
			}
		}
	}

	auto d_buffer = device->createDynamicUniformBuffer(sizeof(glm::mat4), dynamicData.size());

	d_buffer->upload<glm::mat4>(dynamicData);

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

	auto vertexShader = parser.compileVertexShader(readFile("shaders/mvpTexShader.vert"), "main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shaders/mvpTexShader.frag"), "main");
	auto program = device->createShaderProgram(*vertexShader, *fragmentShader);
	auto renderPass = device->createRenderPass(*program, 800, 600, swapchain->getFormat());

	using Clock = std::chrono::high_resolution_clock;
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;

	size_t threadCount = 1;
	ThreadPool threadPool = { threadCount };

	int width, height, comp;
	auto pixels = stbi_load("textures/BYcheckers.png", &width, &height, &comp, 4);

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
					modelMat *= rotateMat;
				}
				d_buffer->upload(dynamicData);
			}

			commandBuffer->record(*renderPass, *swapchain, [&](IRecordingCommandBuffer& rCommandBuffer) {
				//cube->use(rCommandBuffer);
				rCommandBuffer.clearColorBuffer(1.0f, 0.0f, 0.0f, 1.0f);
				rCommandBuffer.execute(subCommands);
			});

			std::vector<std::reference_wrapper<ICommandBuffer>> submissions;
			for (auto& commandBuffer : commandBuffers)
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

void shadow_mapping() {
	const size_t windowWidth = 800;
	const size_t windowHeight = 600;
	auto hwnd = StartWindow(windowWidth, windowHeight);
	auto surface = ISurface::createWin32Surface(windowWidth, windowHeight, hwnd);
	IDevice::Features features{};
	features.samplerAnisotropy = true;
	IDevice::Extensions extensions{};
	extensions.swapchain = true;
	auto devices = IDevice::enumerateDevices(*surface, features, extensions);
	auto& device = devices[0];


	auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe");
	auto swapchain = device->createSwapChain(Format::eR8G8B8A8Unorm, Format::eD32Sfloat, 3, IDevice::PresentMode::eMailbox);
	auto graphicsQueue = device->createGraphicsQueue(*swapchain);
	auto vpMatrixLight = device->createUniformBuffer(sizeof(glm::mat4));
	auto vpMatrixCam = device->createUniformBuffer(sizeof(glm::mat4));

	std::vector<glm::mat4> dynamicData;
	dynamicData.push_back({ glm::translate(glm::vec3{ 0, 0, 0 }) * glm::rotate(0.75f, glm::vec3(0.0f, -1.0f, 0.0f))});
	dynamicData.push_back({ glm::translate(glm::vec3{ 0, 5.0f , 0 }) * glm::scale(glm::vec3(50.0f, 1.0f, 50.0f))});

	auto d_buffer = device->createDynamicUniformBuffer(sizeof(glm::mat4), dynamicData.size());

	d_buffer->upload<glm::mat4>(dynamicData);

	auto cube = std::make_shared<Mesh>(Mesh::Cube(*device));

	{
		glm::mat4 view = glm::lookAt(
			/* eye */	glm::vec3(5.0f, -3.0f, 0.0f),
			/* center */glm::vec3(0.0f, 0.0f, 0.0f),
			/* up */	glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 projection = glm::ortho(-30.0f, 30.0f, 30.0f, -30.0f, 1.0f, 10.0f);

		vpMatrixLight->upload<glm::mat4>({
			projection * view
		});
	}

	auto camPos = glm::vec4(0.0f, -1.0f, 5.0f, 1.0f);
	{
		glm::mat4 view = glm::lookAt(
			glm::vec3(camPos.x, camPos.y, camPos.z),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 projection = glm::perspective(
			glm::radians(90.0f),
			surface->getWidth() * 1.0f / surface->getHeight(),
			1.0f,
			2500.0f);

		vpMatrixCam->upload<glm::mat4>({
			projection * view
		});
	}

	//Renderpass 1 - from lightsource
	auto vertexShader = parser.compileVertexShader(readFile("shaders/mvpTexShader.vert"), "main");
	auto fragmentShader = parser.compileFragmentShader(readFile("shaders/mvpTexShader.frag"), "main");
	auto program = device->createShaderProgram(*vertexShader, *fragmentShader);
	auto shadowPass = device->createRenderPass(*program, 1024, 1024, Format::eB8G8R8A8Unorm, Format::eD32Sfloat);

	// Renderpass 2
	auto shadowVert = parser.compileVertexShader(readFile("shaders/shadow.vert"), "main");
	auto shadowFrag = parser.compileFragmentShader(readFile("shaders/shadow.frag"), "main");
	auto shadowProgram = device->createShaderProgram(*shadowVert, *shadowFrag);

	auto renderpass = device->createRenderPass(*shadowProgram, 800, 600, swapchain->getFormat(), Format::eD32Sfloat);
	

	using Clock = std::chrono::high_resolution_clock;
	auto lastUpdate = Clock::now();
	auto lastFrame = Clock::now();
	long fps = 0;

	int width, height, comp;
	auto pixels = stbi_load("textures/BYcheckers.png", &width, &height, &comp, 4);

	auto imageData = std::vector<char>(width * height * 4);
	memcpy(imageData.data(), pixels, width* height * 4);
	auto image = device->createTexture2D(width, height, Format::eR8G8B8A8Unorm);
	image->upload(imageData);

	auto shadowMap = device->createDepthTexture2D(1024, 1024, Format::eD32Sfloat);
	auto shadowCol = device->createTexture2D(1024, 1024, Format::eB8G8R8A8Unorm);

	auto sam = device->createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirroredRepeat);

	shadowPass->bindResource("view_projection_matrix", *vpMatrixLight);
	shadowPass->bindResource("model_matrix", *d_buffer);
	shadowPass->bindResource("sam", *image, *sam);

	std::vector<std::unique_ptr<ISubCommandBuffer>> shadowSubCommands;
	shadowSubCommands.emplace_back(device->createSubCommandBuffer());
	shadowSubCommands.back()->record(*shadowPass, [&](IRecordingSubCommandBuffer& rscmd) {
		rscmd.setVertexBuffer(*cube->vertex_buffer);
		rscmd.setIndexBuffer(*cube->index_buffer);
		rscmd.setDynamicIndex("model_matrix", 0);
		rscmd.drawIndexed(cube->index_count);
		rscmd.setDynamicIndex("model_matrix", 1);
		rscmd.drawIndexed(cube->index_count);
	});
	

	// commandBuffer->record(*shadowPass, *shadowCol, *shadowMap, [&](IRecordingCommandBuffer& rcmd) {
	// 	rcmd.clearDepthBuffer(1.0f);
	// 	rcmd.execute(subCommands);
	// });

	// graphicsQueue->submitCommands({ *commandBuffer });
	// graphicsQueue->present();
	// device->waitIdle();
 
	// subCommands.clear();

	renderpass->bindResource("view_projection", *vpMatrixCam);
	renderpass->bindResource("shadow_view_projection", *vpMatrixLight);
	renderpass->bindResource("tex", *image, *sam);
	renderpass->bindResource("shadow_map", *shadowMap, *sam);
	renderpass->bindResource("model", *d_buffer);

	std::vector<std::unique_ptr<ISubCommandBuffer>> subCommands;
	subCommands.push_back(device->createSubCommandBuffer());	
	subCommands.back()->record(*renderpass, [&](IRecordingSubCommandBuffer& rSubCmd) {
		rSubCmd.setVertexBuffer(*cube->vertex_buffer);
		rSubCmd.setIndexBuffer(*cube->index_buffer);
		rSubCmd.setDynamicIndex("model", 0);
		rSubCmd.drawIndexed(cube->index_count);
		rSubCmd.setDynamicIndex("model", 1);
		rSubCmd.drawIndexed(cube->index_count);
	});
	
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

			dynamicData[0] *= glm::rotate(0.01f, glm::vec3{1.0f, 0.0f, 0.0f});
			dynamicData[0] *= glm::rotate(0.02f, glm::vec3{0.0f, 1.0f, 0.0f});
			dynamicData[0] *= glm::rotate(0.03f, glm::vec3{0.0f, 0.0f, 1.0f});
			d_buffer->upload(dynamicData);
			camPos = glm::rotate(0.01f, glm::vec3{0.0f, 1.0f, 0.0f}) * camPos;
			{
				glm::mat4 view = glm::lookAt(
					glm::vec3(camPos.x, camPos.y, camPos.z),
					glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(0.0f, 1.0f, 0.0f));

				glm::mat4 projection = glm::perspective(
					glm::radians(90.0f),
					surface->getWidth() * 1.0f / surface->getHeight(),
					1.0f,
					2500.0f);

				vpMatrixCam->upload<glm::mat4>({
					projection * view
				});
			}


			std::vector<std::reference_wrapper<ICommandBuffer>> commandBuffers;
			auto shadowCmd = device->createCommandBuffer();
			shadowCmd->record(*shadowPass, *shadowCol, *shadowMap, [&](IRecordingCommandBuffer& ircb)
			{
				ircb.clearDepthBuffer(1.0f);
				ircb.execute(shadowSubCommands);
			});
			commandBuffers.push_back(*shadowCmd);
			auto commandBuffer = device->createCommandBuffer();
			commandBuffer->record(*renderpass, *swapchain, [&](IRecordingCommandBuffer& rCommandBuffer) {
				//cube->use(rCommandBuffer);
				rCommandBuffer.clearDepthBuffer(1.0f);
				rCommandBuffer.clearColorBuffer(0.0f, 0.5f, 0.7f, 1.0f);
				rCommandBuffer.execute(subCommands);
			});
			commandBuffers.push_back(*commandBuffer);


			graphicsQueue->submitCommands(commandBuffers);

			graphicsQueue->present();
			fps++;
		}
	}
	device->waitIdle();
}

int main()
{	
	//example_triangle();
	//example_cubes();
	//example_cubes_parallel();
	shadow_mapping();
}