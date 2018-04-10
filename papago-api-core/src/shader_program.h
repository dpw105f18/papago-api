#pragma once
#include "standard_header.hpp"

class VertexShader;
class FragmentShader;

class ShaderProgram
{
public:
private:
	ShaderProgram(const vk::UniqueDevice& device, VertexShader& vertexShader, FragmentShader& fragmentShader);
	vk::UniqueShaderModule m_vkVertexModule;
	vk::UniqueShaderModule m_vkFragmentModule;
	vk::PipelineShaderStageCreateInfo m_vkVertexStageCreateInfo;
	vk::PipelineShaderStageCreateInfo m_vkFragmentStageCreateInfo;

	friend class Device;
	friend class RenderPass;
};