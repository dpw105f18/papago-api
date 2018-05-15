#include "test.h"

#include <memory>
#include <future>

//GLM - for glsl types in user code:
#define GLM_ENABLE_EXPERIMENTAL
#include "external/glm/glm.hpp"
#include "external/glm/gtx/transform.hpp"

//File read functions (readFile(...) and readPixels(...))
#include "util.h"

//window stuff
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//API::
#include "external/papago/papago.hpp"

//Vertex data and structure:
#include "mesh.h"

//Thread pool
#include "thread_pool.h"

//convenience:
#define PARSER_COMPILER_PATH "C:/VulkanSDK/1.0.65.0/Bin/glslangValidator.exe"
using CommandBufferRefCollection = std::vector<std::reference_wrapper<ICommandBuffer>>;


void Test::Init(const HWND &win)
{
	auto surface = ISurface::createWin32Surface(800, 600, win);
	auto devices = IDevice::enumerateDevices(*surface, {}, {});
	auto &device = devices[0];
	
	//auto verdat = device->createVertexBuffer(std::vector<glm::vec3>{
	//	//bottom left
	//	{-1.0, 1.0, 0.0},
	//	//bottom right
	//	{1.0, 1.0, 0.0},
	//	//top
	//	{0.0, -1.0, 0.0}
	//});
	auto cube = Mesh::Cube(*device);
	auto &verdat = cube.vertex_buffer;

	auto pars = Parser::Parser(PARSER_COMPILER_PATH);
	auto svert = pars.compileVertexShader(readFile("shaders/mvpTexShader.vert"), "main");
	auto sfrag = pars.compileFragmentShader(readFile("shaders/mvpTexShader.frag"), "main");
	auto shad = device->createShaderProgram(*svert, *sfrag);
	auto firstpass = device->createRenderPass(*shad, 800, 600, Format::eB8G8R8A8Unorm);
	
	auto mod = device->createUniformBuffer(sizeof(glm::mat4));
	auto mat = glm::mat4(1.0f);
	mod->upload<glm::mat4>({ mat });
	firstpass->bindResource("model_matrix", *mod);

	auto proj = device->createUniformBuffer(sizeof(glm::mat4));
	mat = glm::mat4(1.0f);
	proj->upload<glm::mat4>({ mat });
	firstpass->bindResource("view_projection_matrix", *proj);

	auto color = device->createTexture2D(64, 64, Format::eB8G8R8A8Unorm);
	auto color2 = device->createTextureSampler2D(Filter::eNearest, Filter::eNearest, TextureWrapMode::eClampToEdge, TextureWrapMode::eClampToEdge);
	color->upload(readPixels());
	firstpass->bindResource("sam", *color, *color2);

	auto sc = device->createSwapChain(Format::eB8G8R8A8Unorm, 3, IDevice::PresentMode::eMailbox);

	auto cmdBuf = device->createCommandBuffer();
	std::vector<std::unique_ptr<ISubCommandBuffer>> isubvec;
	auto subCmdBuf = device->createSubCommandBuffer();

	subCmdBuf->record(*firstpass, [&](IRecordingSubCommandBuffer &subbuf) {
		subbuf.setVertexBuffer(*verdat);
		subbuf.setIndexBuffer(*cube.index_buffer);
		subbuf.drawIndexed(cube.index_count);
	});

	isubvec.push_back(std::move(subCmdBuf));
	cmdBuf->record(*firstpass, *sc, [&](IRecordingCommandBuffer &recbuf) {
		recbuf.clearColorBuffer(0, 255, 0, 255);
		recbuf.execute(isubvec);
	});
	
	this->GQ = device->createGraphicsQueue(*sc);
	GQ->submitCommands({ *cmdBuf });
	GQ->present();
}

void Test::Loop()
{
	
}
