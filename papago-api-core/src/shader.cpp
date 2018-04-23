#include "standard_header.hpp"
#include "shader.hpp"
#include <fstream>

Shader::Shader(const std::vector<char> & bytecode, std::string entryPoint) : m_entryPoint(entryPoint)
{
	m_code = bytecode;
}

std::vector<Binding> Shader::getBindings() const
{
	auto result = std::vector<Binding>();
	result.reserve(m_bindings.size());
	for (auto& pair : m_bindings) {
		result.push_back(pair.second);
	}
	return result;
}

bool Shader::bindingExists(const std::string & name)
{
	return m_bindings.find(name) != m_bindings.end();
}
