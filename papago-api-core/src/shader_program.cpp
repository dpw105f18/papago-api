#include "standard_header.hpp"
#include "shader_program.hpp"
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

const std::set<uint32_t>& ShaderProgram::getUniqueUniformBindings()
{
	if (m_uniqueBindings.empty()) {

		auto& vBindings = m_vertexShader.getBindings();
		auto& fBindings = m_fragmentShader.getBindings();

		for (auto& vb : vBindings) {
			m_uniqueBindings.insert(vb.binding);
		}

		for (auto& fb : fBindings) {
			m_uniqueBindings.insert(fb.binding);
		}
	}

	return m_uniqueBindings;
}

uint32_t ShaderProgram::getOffset(const std::string & name) const
{
	bool found = false;

	Binding binding;
	if (m_vertexShader.bindingExists(name)) {
		binding = m_vertexShader.m_bindings[name];
	}
	else if (m_fragmentShader.bindingExists(name)) {
		binding = m_fragmentShader.m_bindings[name];
	}

	return binding.offset;
}
