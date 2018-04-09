#include "standard_header.hpp"
#include "vertex_shader.hpp"

VertexShader::VertexShader(const vk::UniqueDevice & device, const std::string & filePath, const std::string& entryPoint) 
	: Shader(device, filePath, entryPoint)
{
	m_vkStageCreateInfo.setModule(m_vkShader.get())
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setPName(m_entryPoint.c_str());
}
