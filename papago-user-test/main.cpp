//FPS counting and displaying in window:
#include <chrono>
#include <sstream>

//GLM - for glsl types in user code:
#define GLM_ENABLE_EXPERIMENTAL
#include "external/glm/glm.hpp"
#include "external/glm/gtx/transform.hpp"

//File read functions (readFile(...) and readPixels(...))
#include "util.h"

//API::
#include "external/papago/papago.hpp"

//convenience:
#define PARSER_COMPILER_PATH "C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe"
using CommandBufferRefCollection = std::vector<std::reference_wrapper<ICommandBuffer>>;

struct Vertex
{
	glm::vec3 pos;
	//glm::vec2 uv;
};


int main()
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
		{ {  0.5f, -0.5f, 0.5f } },
		{ { -0.5f, -0.5f, 0.5f } },
		{ {  0.0f,  0.5f, 0.5f } },
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

	CommandBufferRefCollection cmds = { *cmd};

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