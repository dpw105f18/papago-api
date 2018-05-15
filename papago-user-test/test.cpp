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


void Test::Init()
{
}

void Test::Loop()
{
}
