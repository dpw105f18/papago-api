#pragma once
#include "standard_header.hpp"
#include <string>
class Shader {
public:
	Shader(Shader&) = delete;

protected:
	Shader(const vk::UniqueDevice& device, const std::string& filePath);
	Shader(Shader&&) = default;
	vk::UniqueShaderModule m_vkShader;
	vk::PipelineShaderStageCreateInfo m_vkStageCreateInfo;	//<-- set by children

private:
	std::vector<char> readFile(const std::string& filePath);
};