#pragma once
#include <set>
#include "ishader_program.hpp"

class VertexShader;
class FragmentShader;
class CommandBuffer;

class ShaderProgram : public IShaderProgram
{
public:
	ShaderProgram(const vk::UniqueDevice& device, VertexShader& vertexShader, FragmentShader& fragmentShader);
	vk::UniqueShaderModule m_vkVertexModule;
	vk::UniqueShaderModule m_vkFragmentModule;
	vk::PipelineShaderStageCreateInfo m_vkVertexStageCreateInfo;
	vk::PipelineShaderStageCreateInfo m_vkFragmentStageCreateInfo;
	const std::set<uint32_t>& getUniqueUniformBindings();
	uint32_t getOffset(const std::string& name) const;

	VertexShader& m_vertexShader;
	FragmentShader& m_fragmentShader;

private:
	std::set<uint32_t> m_uniqueBindings;
};