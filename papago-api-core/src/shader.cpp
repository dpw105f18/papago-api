#include "standard_header.hpp"
#include "shader.hpp"
#include <fstream>

Shader::Shader(const std::string & filePath, std::string entryPoint) : m_entryPoint(entryPoint)
{
	m_code = readFile(filePath);
}

//if the return is empty, then an PAPAGO_ERROR might have occured
std::vector<char> Shader::readFile(const std::string & filePath)
{
	std::vector<char> result;
	
	auto file = std::ifstream(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		PAPAGO_ERROR("could not open file: " + filePath);
	}
	else {
		size_t fileSize = static_cast<size_t>(file.tellg());
		result.resize(fileSize);
		file.seekg(0);
		file.read(result.data(), fileSize);
	}

	return result;
}
