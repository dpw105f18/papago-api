#pragma once
#include "standard_header.hpp"
#include <string>
class Shader {
public:
	Shader(Shader&) = delete;

protected:
	Shader(const vk::UniqueDevice& device, const std::string& filePath, std::string entryPoint);
	Shader(Shader&&);
	vk::UniqueShaderModule m_vkShader;
	vk::PipelineShaderStageCreateInfo m_vkStageCreateInfo;	//<-- set by children
	std::string m_entryPoint;

private:
	std::vector<char> readFile(const std::string& filePath);	//TODO: move to a Parser-stub
};