#include "standard_header.hpp"
#include "parser.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include <sstream>
#include <regex>
#include <map>


Parser::Parser(const std::string & compilePath): m_compilePath(compilePath)
{
}

#define STUPID_VERTEX_SHADER_HASH 0xa709c4e0b8ab6894
#define COLOR_VERTEX_SHADER_HASH 0xf454a08ee86af30a

std::unique_ptr<IVertexShader> Parser::compileVertexShader(const std::string &source, const std::string &entryPoint)
{
	auto byte_code = compile(source, "vert");
	auto result = std::make_unique<VertexShader>(byte_code, entryPoint);

	setShaderInput(*result, source);
	setShaderUniforms(*result, source);

	return result;
}

#define STUPID_FRAGMENT_SHADER_HASH 0xfc0838ff5f18bfc2
#define COLOR_FRAGMENT_SHADER_HASH 0xb810ed8016ecd8d6

std::unique_ptr<IFragmentShader> Parser::compileFragmentShader(const std::string& source, const std::string& entryPoint)
{
	auto byte_code = compile(source, "frag");
	auto result = std::make_unique<FragmentShader>(byte_code, entryPoint);

	setShaderUniforms(*result, source);
	
	return result;
}

std::vector<char> Parser::compile(const std::string& source, const std::string& shaderType) const
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
		char buffer[2048] = {};
		DWORD bytes_read;
		ReadFile(stdout_read, &buffer, 2048, &bytes_read, nullptr);
		std::cout << buffer << std::endl;
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

size_t string_type_to_size(std::string type) {
	static const std::map<std::string, size_t> map{
		{ "float",     sizeof(float) },
		{ "vec2" ,   2*sizeof(float) },
		{ "vec3" ,   3*sizeof(float) },
		{ "vec4" ,   4*sizeof(float) },
		{ "mat4" , 4*4*sizeof(float) }
	};
	auto result = map.find(type);
	if (result == map.end()) {
		PAPAGO_ERROR("Failed to convert " + type + " to a size.");
	}
	return result->second;
}

vk::Format string_type_to_format(std::string type) {
	static const std::map<std::string, vk::Format> map{
		{ "float", vk::Format::eD32Sfloat },
		{ "vec2", vk::Format::eR32G32Sfloat },
		{ "vec3", vk::Format::eR32G32B32Sfloat },
		{ "vec4", vk::Format::eR32G32B32A32Sfloat },
		{ "mat4", vk::Format::eUndefined}
	};
	auto result = map.find(type);
	if (result == map.end()) {
		PAPAGO_ERROR("Failed to convert " + type + " to a format.");
	}
	return result->second;
}

#define REGEX_NUMBER "[0-9]+"
#define REGEX_NAME "[a-zA-Z_][a-zA-Z0-9_]*"

void Parser::setShaderInput(VertexShader & shader, const std::string & source)
{
	static const auto regex = std::regex(".*layout\\s*\\(location\\s*=\\s*(" REGEX_NUMBER ")\\s*\\)\\s+in\\s+(" REGEX_NAME ")\\s+(" REGEX_NAME ");");
	

	std::sregex_iterator iterator(ITERATE(source), regex);
	for (auto i = iterator; i != std::sregex_iterator(); ++i) {
		auto match = *i;
		auto location = std::stoi(match[1]);
		auto format = string_type_to_format(match[2].str());
		auto name = match[3].str();

		if (shader.m_input.size() <= location) {
			shader.m_input.resize(location+1);
		}
		shader.m_input[location] = { 0, format };
	}

	// Calculate offsets. Can't do it in loop above, as allocation order could be mixed.
	auto offset = 0u;
	for (auto& input : shader.m_input) {
		input.offset = offset;
		offset += input.getFormatSize();
	}

}

void Parser::setShaderUniforms(Shader & shader, const std::string & source)
{
	static const auto blockRegex = std::regex("layout\\s*\\(binding\\s*=\\s*(" REGEX_NUMBER ")\\s*\\)\\s*uniform\\s+" REGEX_NAME "\\s*\\{([^\\}]*)\\}\\s*" REGEX_NAME "\\s*;");

	for (std::sregex_iterator iterator(ITERATE(source), blockRegex); iterator != std::sregex_iterator(); ++iterator) {
		auto match = *iterator;
		uint32_t binding = std::stoi(match[1].str());
		auto body = match[2].str();

		auto offset = 0u;
		static const auto body_regex = std::regex("(" REGEX_NAME ")\\s+(" REGEX_NAME ");");
		for (std::sregex_iterator body_iterator(ITERATE(body), body_regex); body_iterator != std::sregex_iterator(); ++body_iterator) {
			auto body_match = *body_iterator;
			auto type = body_match[1];
			auto name = body_match[2];
				
			vk::DescriptorType descriptorType;
			auto typeByteSize = 0u;

			descriptorType = vk::DescriptorType::eUniformBufferDynamic;
			typeByteSize = string_type_to_size(type);
			

			shader.m_bindings.insert({ name, { binding, offset, descriptorType } });
			offset += typeByteSize;
		}
	}

	static const auto regex = std::regex("layout\\s*\\(binding\\s*=\\s*(" REGEX_NUMBER ")\\s*\\)\\s*uniform\\s+(" REGEX_NAME ")\\s+(" REGEX_NAME ")\\s*;");

	for (std::sregex_iterator iterator(ITERATE(source), regex); iterator != std::sregex_iterator(); ++iterator) {
		auto match = *iterator;
		uint32_t binding = std::stoi(match[1]);
		auto type = match[2];
		auto name = match[3];

		// TODO: Expand to 1D and 3D samplers? - Brandborg
		vk::DescriptorType descriptorType = type == std::string("sampler2D")
			? vk::DescriptorType::eCombinedImageSampler
			: vk::DescriptorType::eUniformBuffer; // Technically a buffer uniform cannot be found in the free like this.

		shader.m_bindings.insert({ name,{ binding, 0, descriptorType } });
	}
}
