#include "standard_header.hpp"
#include "parser.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include <sstream>

Parser::Parser(const std::string & compilePath): m_compilePath(compilePath)
{
}

#define STUPID_VERTEX_SHADER_HASH 0xa709c4e0b8ab6894
#define COLOR_VERTEX_SHADER_HASH 0xf454a08ee86af30a
#define TEXTURE_VERTEX_SHADER_HASH 0x19e592a304b02230

std::unique_ptr<IVertexShader> Parser::compileVertexShader(const std::string &source, const std::string &entryPoint)
{
	auto byte_code = compile(source, "vert");
	auto result = std::make_unique<VertexShader>(byte_code, entryPoint);
	std::hash<std::string> hashFun;
	auto hash = hashFun(source);


	if (hash == STUPID_VERTEX_SHADER_HASH) {
		result->m_input.push_back({ 0, vk::Format::eR32G32B32Sfloat });	//<-- position
	}
	else if(hash == COLOR_VERTEX_SHADER_HASH){
		result->m_input.push_back({ 0, vk::Format::eR32G32B32Sfloat });	//<-- position
		result->m_input.push_back({ sizeof(float) * 3, vk::Format::eR32G32Sfloat }); //<-- uv
	}
	else if (hash == TEXTURE_VERTEX_SHADER_HASH) {
		result->m_input.push_back({ 0, vk::Format::eR32G32B32Sfloat });	//<-- position
	}
	return result;
}

#define STUPID_FRAGMENT_SHADER_HASH 0xfc0838ff5f18bfc2
#define COLOR_FRAGMENT_SHADER_HASH 0xb810ed8016ecd8d6
#define TEXTURE_FRAGMENT_SHADER_HASH 0xdc0a529a31085233

std::unique_ptr<IFragmentShader> Parser::compileFragmentShader(const std::string& source, const std::string& entryPoint)
{
	auto byte_code = compile(source, "frag");
	auto result = std::make_unique<FragmentShader>(byte_code, entryPoint);
	
	std::hash<std::string> hashFun;
	auto hash = hashFun(source);

	//TODO: set binding information on [result]
	
	// For texture frag
	//result.m_bindings.insert({ "texSampler", {0, vk::DescriptorType::eCombinedImageSampler} });
	
	// For uniform frag
	if (hash == COLOR_FRAGMENT_SHADER_HASH) {
	
	}
	else if(hash == STUPID_FRAGMENT_SHADER_HASH){
		result->m_bindings.insert({ { "sam" },{ 0, vk::DescriptorType::eCombinedImageSampler } });
		result->m_bindings.insert({ { "val" },{ 1, vk::DescriptorType::eUniformBuffer } });
	}
	else if (hash == TEXTURE_FRAGMENT_SHADER_HASH) {
		result->m_bindings.insert({ {"texSampler"}, {0, vk::DescriptorType::eCombinedImageSampler} });
	}
	
	return result;
}

std::vector<char> Parser::compile(const std::string& source, const std::string& shaderType)
{
	auto arg = std::string(" --stdin -S ") + shaderType
		+ std::string(" -V -o ") + std::string(".\\temp.spv");

	// Create a pipe between the handles 'read' and 'write', so that anything 
	// written to 'write' can be read by 'read'.
	HANDLE stdin_read, stdin_write, stdout_read, stdout_write;

	SECURITY_ATTRIBUTES security_attributes = {};
	security_attributes.bInheritHandle = true;
	security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	security_attributes.lpSecurityDescriptor = nullptr;

	if (!CreatePipe(&stdin_read, &stdin_write, &security_attributes, 0))
		PAPAGO_ERROR("Could not create pipe.");
	if (!SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0))
		PAPAGO_ERROR("Stdout SetHandleInformation");

	if (!CreatePipe(&stdout_read, &stdout_write, &security_attributes, 0))
		PAPAGO_ERROR("Could not create pipe.");
	if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0))
		PAPAGO_ERROR("Stdout SetHandleInformation");

	// Tell the process to use the 'read' handle as its stdin
	STARTUPINFO startUpInfo = {};
	startUpInfo.cb = sizeof(startUpInfo);
	startUpInfo.hStdInput = stdin_read;
	startUpInfo.hStdOutput = stdout_write;
	startUpInfo.hStdError = stdout_write;
	startUpInfo.dwFlags |= STARTF_USESTDHANDLES;

	PROCESS_INFORMATION processInfo;

	if (!CreateProcess(m_compilePath.c_str(),
		&arg[0],
		nullptr,
		nullptr,
		true,
		0,
		nullptr,
		nullptr,
		&startUpInfo,
		&processInfo))
	{
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		DeleteFile(".\\temp.spv");
		PAPAGO_ERROR("Could not start compilation process.");
	}

	DWORD bytesWritten;
	WriteFile(stdin_write, source.c_str(), source.size(), &bytesWritten, nullptr);
	// Write end of input token to handle
	WriteFile(stdin_write, "\n\x1a", 2, &bytesWritten, nullptr);

	WaitForSingleObject(processInfo.hThread, 1000);

	DWORD exit_code;
	if (!GetExitCodeProcess(processInfo.hProcess, &exit_code))
		PAPAGO_ERROR("Failed to get exit code from child process.");

	if (exit_code != EXIT_SUCCESS) {
		char buffer[2048];
		DWORD bytes_read;
		ReadFile(stdout_read, &buffer, 2048, &bytes_read, nullptr);

		PAPAGO_ERROR("Validator could not validate input.");
	}

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
	CloseHandle(stdin_read);
	CloseHandle(stdin_write);
	CloseHandle(stdout_read);
	CloseHandle(stdout_write);

	auto file = CreateFile(
		".\\temp.spv",
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
	if (file == INVALID_HANDLE_VALUE)
		PAPAGO_ERROR("Could not open file.");

	auto size = GetFileSize(file, nullptr);

	std::vector<char> buffer(size);
	ReadFile(file, buffer.data(), size, nullptr, nullptr);

	CloseHandle(file);
	DeleteFile(".\\temp.spv");
	return buffer;
}
