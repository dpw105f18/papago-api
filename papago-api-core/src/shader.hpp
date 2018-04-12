#pragma once
#include "standard_header.hpp"
#include "parser.hpp"
#include <string>
#include <map>

struct Binding
{
	uint32_t binding;
	vk::DescriptorType type;
};

class Shader {
public:

protected:
	Shader(const std::string& filePath, const std::string entryPoint);
	const std::string m_entryPoint;
	std::vector<char> m_code;
	std::map<std::string, Binding> m_bindings;

	std::vector<Binding> getBindings() const;
	bool bindingExists(const std::string& name);
private:
	static std::vector<char> readFile(const std::string& filePath);	//TODO: move to a Parser-stub

	friend class CommandBuffer;
};
