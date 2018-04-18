#include "standard_header.hpp"
#include "parser.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include <sstream>

Parser::Parser(const std::string & compilePath): m_compilePath(compilePath)
{
}

VertexShader Parser::compileVertexShader(const std::string &filePath, const std::string &entryPoint)
{
	auto spvFile = compile(filePath);


	auto result = VertexShader(spvFile, entryPoint);

	//TODO: set binding information on [result]

	if (filePath == "shader/colorVert.vert") {

		result.m_input.push_back({ 0, vk::Format::eR32G32B32Sfloat });	//<-- position
	}
	else if(filePath == std::string("shader/stupidVert.vert")){
		
		result.m_input.push_back({ 0, vk::Format::eR32G32B32Sfloat });	//<-- position
		result.m_input.push_back({ sizeof(float) * 3, vk::Format::eR32G32Sfloat }); //<-- uv
	}
	return result;
}

FragmentShader Parser::compileFragmentShader(const std::string & filePath, const std::string & entryPoint)
{
	auto spvFile = compile(filePath);
	auto result = FragmentShader(spvFile, entryPoint);
	
	//TODO: set binding information on [result]
	
	// For texture frag
	//result.m_bindings.insert({ "texSampler", {0, vk::DescriptorType::eCombinedImageSampler} });
	
	// For uniform frag
	if (filePath == std::string("shader/colorFrag.frag")) {
	
	}
	else if(filePath == std::string("shader/stupidFrag.frag")){
		result.m_bindings.insert({ { "sam" },{ 0, vk::DescriptorType::eCombinedImageSampler } });
		result.m_bindings.insert({ { "val" },{ 1, vk::DescriptorType::eUniformBuffer } });
	}
	
	return result;
}

std::string Parser::compile(const std::string & filePath)
{
	auto pathIndex = filePath.find_last_of('/');
	auto spirVPath = filePath.substr(0, pathIndex);

	auto fileIndex = filePath.find_last_of('.');
	auto fileName = filePath.substr(pathIndex, fileIndex - pathIndex);
	// '/' is already included in the spirVPath and thus is not needed here
	auto spvFile = spirVPath + fileName + ".spv";

	//TODO: append stage to spv filename (so test.vert and test.frag => testVert.spv and testFrag.spv). -AM.
	// Is here or else c_str will point to junk
	auto arg = std::string(" -V ")	//<-- compile using Vulkan semantics
		+ std::string("-o ") + std::string("\"") + spvFile + std::string("\" ") //<-- output file = fileName.spv (in same folder as fileName.vert)
		+ std::string("\"") + filePath + "\" ";	//<-- fileName.vert file

	auto commandLineArguments = LPTSTR(arg.c_str());

	STARTUPINFO startUpInfo;
	PROCESS_INFORMATION processInfo;

	ZeroMemory(&startUpInfo, sizeof(startUpInfo));
	startUpInfo.cb = sizeof(startUpInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));

	auto success = CreateProcess(m_compilePath.c_str(),
		commandLineArguments,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startUpInfo,
		&processInfo);

	if (!success) {
		PAPAGO_ERROR("Could not start compilation process."); //TODO:: Close handle on error? - Brandborg
	}
	else {
		WaitForSingleObject(processInfo.hThread, INFINITE);
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}

	return spvFile;
}
