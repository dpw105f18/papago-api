#include "standard_header.hpp"
#include "vertex_shader.hpp"

VertexShader::VertexShader(const std::vector<char> & bytecode, const std::string& entryPoint) 
	: Shader(bytecode, entryPoint)
{
}

uint32_t VertexShader::Input::getFormatSize()
{
	{
		switch (format) {
		case vk::Format::eR32G32B32Sfloat:
			return sizeof(float) * 3;
			break;
		case vk::Format::eR32G32Sfloat:
			return sizeof(float) * 2;
			break;
		default:
			PAPAGO_ERROR("Format size not implemented!");
		}
	}
}
