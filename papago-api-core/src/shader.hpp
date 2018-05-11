#pragma once
#include <string>
#include <map>

struct Binding
{
	uint32_t binding;
	uint32_t offset;
	vk::DescriptorType type;
};

class Shader {
public:
	Shader(const std::vector<char>& bytecode, const std::string entryPoint);
	const std::string m_entryPoint;
	std::vector<char> m_code;
	std::map<std::string, Binding> m_bindings;

	std::vector<Binding> getBindings() const;
	bool bindingExists(const std::string& name);
private:

};
