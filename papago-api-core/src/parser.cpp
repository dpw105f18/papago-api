#include "standard_header.hpp"
#include "parser.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include <sstream>

Parser::Parser(const std::string & compilePath): m_compilePath(compilePath)
{
}

VertexShader Parser::compileVertexShader(const std::string & filePath)
{


	auto pathIndex = filePath.find_last_of('/');
	auto spirVPath = filePath.substr(0, pathIndex);

	auto fileIndex = filePath.find_last_of('.');
	auto fileName = filePath.substr(pathIndex, fileIndex - pathIndex);
	auto spvFile = spirVPath + "/" + fileName + ".spv";
	
	// Is here or else c_str will point to junk
	auto arg =  std::string(" -V ")	//<-- compile using Vulkan semantics
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


	return VertexShader();
}

/*
FragmentShader Parser::compileFragmentShader(const std::string & filePath)
{
	return FragmentShader();
}*/

