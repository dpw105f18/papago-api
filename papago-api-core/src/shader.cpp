#include "standard_header.hpp"
#include "shader.hpp"
#include <fstream>

Shader::Shader(const vk::UniqueDevice& device, const std::string & filePath, std::string entryPoint ): m_entryPoint(entryPoint)
{
	auto code = readFile(filePath);
	vk::ShaderModuleCreateInfo createInfo = {};
	createInfo.setCodeSize(code.size())
		.setPCode(reinterpret_cast<const uint32_t*>(code.data()));

	m_vkShader = device->createShaderModuleUnique(createInfo);
}

Shader::Shader(Shader &&other) noexcept
	: m_entryPoint(std::move(other.m_entryPoint))
	, m_vkStageCreateInfo(std::move(other.m_vkStageCreateInfo))
	, m_vkShader(std::move(other.m_vkShader))
{
	m_vkStageCreateInfo.setPName(m_entryPoint.c_str());
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
