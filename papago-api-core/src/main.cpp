#include "standard_header.hpp"
#include "device.hpp"
#include "swap_chain.hpp"
#include "surface.hpp"
#include "sampler.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "render_pass.hpp"
#include "graphics_queue.hpp"
#include "command_buffer.hpp"
#include "parser.hpp"
#include <WinUser.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


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

struct UniformBufferObject{};

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
	vec3 m_position;
	vec2 m_uv;
};


ImageResource createTexture(Device& device) {
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
	imageResource.upload(input);

	return imageResource;
}

int main()
{
	auto hwnd = StartWindow(800, 600);
	auto surface = Surface(800, 600, hwnd);
	Features features = {};
	features.samplerAnisotropy = true;
	auto devices = Device::enumerateDevices(surface, features, { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME });
	auto& device = devices[0];

	auto stupidVertices = std::vector<Vertex>{
		{ { -0.5, -0.5, 0.5 },{ 0.0, 0.0 } },
		{ { -0.5,  0.5, 0.5 },{ 0.0, 1.0 } },
		{ { 0.5,  0.5, 0.5 },{ 1.0, 1.0 } },
		{ { 0.5, -0.5, 0.5 },{ 1.0, 0.0 } }
	};
	auto stupidVertexBuffer = device.createVertexBuffer(stupidVertices);

	auto vertices = std::vector<vec3>{
		{-0.5, -0.5, 0.5},
		{-0.5,  0.5, 0.5},
		{ 0.5,  0.5, 0.5},
		{ 0.5, -0.5, 0.5}
	};
	auto vertexBuffer = device.createVertexBuffer(vertices);

	auto indices = std::vector<uint16_t>{
		0, 1, 2,
		0, 2, 3
	};
	auto indexBuffer = device.createIndexBuffer(indices);

	// PASS 1
	auto parser = Parser("C:/VulkanSDK/1.0.65.0/Bin32/glslangValidator.exe");
	auto colVert = parser.compileVertexShader("shader/colorVert.vert", "main");
	auto colFrag = parser.compileFragmentShader("shader/colorFrag.frag", "main");

	auto colProgram = device.createShaderProgram(colVert, colFrag);

	auto passOneTarget = device.createTexture2D(800, 600, Format::eR8G8B8A8Unorm);

	auto colPass = device.createRenderPass(colProgram, passOneTarget.getWidth(), passOneTarget.getHeight(), passOneTarget.getFormat(), false);

	
	// PASS 2
	auto swapChain = device.createSwapChain(Format::eR8G8B8A8Unorm, 3, SwapChainPresentMode::eMailbox);

	auto graphicsQueue = device.createGraphicsQueue(swapChain);
	
	auto stupidVert = parser.compileVertexShader("shader/stupidVert.vert", "main");
	auto stupidFrag = parser.compileFragmentShader("shader/stupidFrag.frag", "main");

	auto stupidProgram = device.createShaderProgram(stupidVert, stupidFrag);
	auto stupidPass = device.createRenderPass(stupidProgram, swapChain.getWidth(), swapChain.getHeight(), swapChain.getFormat(), true);



	auto uniform_buffer = device.createUniformBuffer<sizeof(float[3])>();
	auto sampler2D = device.createTextureSampler2D(Filter::eLinear, Filter::eLinear, TextureWrapMode::eMirroredRepeat, TextureWrapMode::eMirrorClampToEdge);

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
			cmd.begin(colPass, passOneTarget);

			cmd.setInput(vertexBuffer);
			cmd.setIndexBuffer(indexBuffer);
			cmd.drawIndexed(indices.size());

			cmd.end();

			auto stupidCmd = device.createCommandBuffer(Usage::eReuse);
			stupidCmd.begin(stupidPass, swapChain, graphicsQueue.getCurrentFrameIndex());

			auto uniform_input_float = std::vector<float>({ std::rand() * 1.0f / RAND_MAX, std::rand() * 1.0f / RAND_MAX, std::rand() * 1.0f / RAND_MAX });
			auto uniform_input_char = std::vector<char>(sizeof(float) * uniform_input_float.size());

			// TODO: move into upload as template???
			memcpy(uniform_input_char.data(), uniform_input_float.data(), uniform_input_char.size());

			if (!uniform_buffer.inUse()) {
				uniform_buffer.upload(uniform_input_char);
			}
			else {
				auto d = "bug";
			}

			stupidCmd.setUniform("val", uniform_buffer);
			stupidCmd.setUniform("sam", passOneTarget, sampler2D);

			stupidCmd.setInput(stupidVertexBuffer);
			stupidCmd.setIndexBuffer(indexBuffer);
			stupidCmd.drawIndexed(indices.size());
			stupidCmd.end();


			std::vector<CommandBuffer> cmds;
			cmds.push_back(std::move(cmd));
			cmds.push_back(std::move(stupidCmd));
			graphicsQueue.submitCommands(cmds);

			graphicsQueue.present();
			graphicsQueue.wait();

			/*
			if (!passOneTarget.inUse()) {
				auto data = passOneTarget.download();
				auto d = "bug";
			}
			else {
				auto d = "bug";
			}
			*/
		}
	}//END while
	std::cin.ignore();
}
