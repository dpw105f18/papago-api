#include "standard_header.hpp"
#include "fragment_shader.hpp"

FragmentShader::FragmentShader(const vk::UniqueDevice & device, const std::string & filePath, const std::string & entryPoint)
	: Shader(device, filePath, entryPoint)
{
	m_vkStageCreateInfo.setModule(*m_vkShader)
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPName(entryPoint.c_str());
}
