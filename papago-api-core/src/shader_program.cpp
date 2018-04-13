#include "standard_header.hpp"
#include "shader_program.h"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "command_buffer.hpp"

ShaderProgram::ShaderProgram(const vk::UniqueDevice& device, VertexShader& vertexShader, FragmentShader& fragmentShader)
	: m_vertexShader(vertexShader), m_fragmentShader(fragmentShader)
{
	//Module:
	vk::ShaderModuleCreateInfo vertexInfo = {};
	vertexInfo.setCodeSize(vertexShader.m_code.size())
		.setPCode(reinterpret_cast<const uint32_t*>(vertexShader.m_code.data()));

	m_vkVertexModule = device->createShaderModuleUnique(vertexInfo);
	
	//Stage create info:
	m_vkVertexStageCreateInfo.setModule(*m_vkVertexModule)
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setPName(vertexShader.m_entryPoint.c_str());


	//Module:
	vk::ShaderModuleCreateInfo fragmentInfo = {};
	fragmentInfo.setCodeSize(fragmentShader.m_code.size())
		.setPCode(reinterpret_cast<const uint32_t*>(fragmentShader.m_code.data()));

	m_vkFragmentModule = device->createShaderModuleUnique(fragmentInfo);

	//Stage create Info:
	m_vkFragmentStageCreateInfo.setModule(*m_vkFragmentModule)
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPName(fragmentShader.m_entryPoint.c_str());

}