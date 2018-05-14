//FPS counting and displaying in window:
#include <chrono>
#include <sstream>
#include <memory>

//GLM - for glsl types in user code:
#define GLM_ENABLE_EXPERIMENTAL
#include "external/glm/glm.hpp"
#include "external/glm/gtx/transform.hpp"

//File read functions (readFile(...) and readPixels(...))
#include "util.h"

//API::
#include "external/papago/papago.hpp"

//Vertex data and structure:
#include "mesh.h"

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


int main()
{	
	//example_triangle();
	example_cubes();
}