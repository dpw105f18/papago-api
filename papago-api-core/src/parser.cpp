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

void Parser::setShaderInput(VertexShader & shader, const std::string & source)
{
	std::vector<std::string> layoutLines;

	auto layoutRegex = std::regex(std::string(".*layout\\s*\\(.*\\)\\s+in.*;"));
	std::smatch layoutMatches;
	auto toSearch = source;

	do {
		if (std::regex_search(toSearch, layoutMatches, layoutRegex)) {
			layoutLines.push_back(layoutMatches[0].str());
			toSearch = layoutMatches.suffix().str();
		}
	} while (layoutMatches.size() > 0);

	std::map<int, std::string> locationTypes;
	for (auto& layoutLine : layoutLines) {
		std::smatch matches;
		if (std::regex_search(layoutLine, matches, std::regex("layout\\s*\\(\\s*location\\s*=\\s*"))) {
			std::string suffix = matches.suffix().str();

			//get location index
			std::regex_search(suffix, matches, std::regex("^[0-9]+"));
			auto location = std::stoi(matches[0].str());

			//get variable name (even though we don't use it, the prefix of this match is used later)
			std::regex_search(suffix, matches, std::regex("([a-zA-Z_][a-zA-Z0-9_\\-]*)(\\s*;)$"));
			auto name = matches[1].str();	/* <-- [0] is a match of the entire string, e.g. "myName ;"
											*     [1] is a match of the first group, e.g. "myName" (with " ;" as [2])
											*/
			
			//get variable type
			auto prefix = matches.prefix().str();
			std::regex_search(prefix, matches, std::regex("([a-zA-Z][a-z-A-Z0-9]+)(\\s)$"));
			auto type = matches[1].str();

			locationTypes.insert({ location, type });
		}
	}

	shader.m_input.resize(locationTypes.size());

	//TODO: can we assume that the locations don't skip an index? -AM
	uint32_t offset = 0;
	for (auto i = 0; i < locationTypes.size(); ++i) {
		//handle type:
		auto& type = locationTypes[i];
		vk::Format format = vk::Format::eUndefined;
		uint32_t formatByteSize = 0;
		//TODO: find better way to get format from types
		if (type == std::string("vec2")) {
			format = vk::Format::eR32G32Sfloat;
			formatByteSize = sizeof(float) * 2;
		}
		else if (type == std::string("vec3")) {
			format = vk::Format::eR32G32B32Sfloat;
			formatByteSize = sizeof(float) * 3;
		}
		else if (type == std::string("vec4")) {
			format = vk::Format::eR32G32B32A32Sfloat;
			formatByteSize = sizeof(float) * 4;
		}

		shader.m_input[i] = { offset, format };
		offset += formatByteSize;
	}
}

void Parser::setShaderUniforms(Shader & shader, const std::string & source)
{
	std::vector<std::string> layoutLines;

	//there are two types of uniforms:
	//block: layout(binding = [index]) uniform [type] { ... } [alias];
	//no-block: layout(binding = [index]) uniform [type] [name];
	constexpr auto uniformRegex = "(\\s+layout\\s*\\(.*\\)\\s+uniform)";	//<-- what they have in common
	constexpr auto noBlockRegex = "(\\s+.+\\s+.+;)";
	constexpr auto blockRegex = "(\\s+.+\\s*\\{[^\\}]*\\}.+;)";

	auto layoutRegex = std::regex(uniformRegex + std::string("(") + blockRegex + "|" + noBlockRegex  + ")");
	std::smatch layoutMatches;
	auto toSearch = source;
	do {
		if (std::regex_search(toSearch, layoutMatches, layoutRegex)) {
			layoutLines.push_back(layoutMatches[0].str());
			toSearch = layoutMatches.suffix().str();
		}
	} while (layoutMatches.size() > 0);

	std::map<std::string, Binding> namedBindings;
	for (auto& layoutLine : layoutLines) {
		std::smatch matches;
		if (std::regex_search(layoutLine, matches, std::regex("layout\\s*\\(\\s*binding\\s*=\\s*"))) {
			std::string suffix = matches.suffix().str();

			//get binding index
			std::regex_search(suffix, matches, std::regex("^[0-9]+"));
			uint32_t binding = std::stoi(matches[0].str());
			uint32_t offset = 0;
			do {
				if (std::regex_search(suffix, matches, std::regex("([a-zA-Z_][a-zA-Z0-9_\\-]*)(\\s*;)"))) {
					//get variable name
					auto name = matches[1].str();	/* <-- [0] is a match of the entire string, e.g. "myName ;"
													*      [1] is a match of the first group, e.g. "myName" (with " ;" as [2])
													*/

					//get variable type
					auto prefix = matches.prefix().str();
					std::regex_search(prefix, matches, std::regex("([a-zA-Z][a-z-A-Z0-9]+)(\\s)$"));
					//is it a variable (true) or a block alias (false)?
					if (matches.size() > 0) {
						auto type = matches[1].str();

						vk::DescriptorType descriptorType;
						uint32_t typeByteSize = 0;

						if (type == std::string("sampler2D")) {
							descriptorType = vk::DescriptorType::eCombinedImageSampler;
						}
						else {
							descriptorType = vk::DescriptorType::eUniformBuffer;

							if (type == std::string("vec2")) {
								typeByteSize = sizeof(float) * 2;
							}
							else if (type == std::string("vec3")) {
								typeByteSize = sizeof(float) * 3;
							}
							else if (type == std::string("vec4")) {
								typeByteSize = sizeof(float) * 4;
							}
							else if (type == std::string("float")) {
								typeByteSize = sizeof(float);
							}
							//TODO: add more types to be recognized by the parser. -AM
						}

						shader.m_bindings.insert({ name, {binding, offset, descriptorType} });
						offset += typeByteSize;

						std::smatch nextSemicolon;
						if (std::regex_search(suffix, nextSemicolon, std::regex(";"))) {
							suffix = nextSemicolon.suffix();
						}
					} //end if match is variable (not block alias)
				} //end if there are more variables to check in this layout line
			} while (matches.size() > 0);
		}//end if used to find where the binding index is in the layout line
	}//end foreach layoutLine
}
