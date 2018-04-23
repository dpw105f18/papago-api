#include "standard_header.hpp"
#include "fragment_shader.hpp"

FragmentShader::FragmentShader(const std::vector<char>& bytecode, const std::string & entryPoint)
	: Shader(bytecode, entryPoint)
{
}
