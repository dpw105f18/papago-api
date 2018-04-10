#pragma once
#include "standard_header.hpp"
#include "parser.hpp"
#include <string>
class Shader {
public:
	Shader(const Shader&) = delete;

protected:
	Shader() {};
	Shader(const vk::UniqueDevice& device, const std::string& filePath, std::string entryPoint);
	Shader(Shader&&) noexcept;
	vk::UniqueShaderModule m_vkShader;
	vk::PipelineShaderStageCreateInfo m_vkStageCreateInfo;	//<-- set by children
	std::string m_entryPoint;

private:
	static std::vector<char> readFile(const std::string& filePath);	//TODO: move to a Parser-stub
};