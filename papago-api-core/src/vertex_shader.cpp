#include "standard_header.hpp"
#include "vertex_shader.hpp"

VertexShader::VertexShader(const std::vector<char> & bytecode, const std::string& entryPoint) 
	: Shader(bytecode, entryPoint)
{
}
