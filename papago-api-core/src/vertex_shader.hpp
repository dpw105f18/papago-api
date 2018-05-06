#pragma once
#include "shader.hpp"
#include "ishader.hpp"

class VertexShader : public Shader, public IVertexShader
{
public:
	VertexShader(const std::vector<char>& bytecode, const std::string& entryPoint);

	struct Input
	{
		uint32_t offset;	//<-- offset in bytes from the beginning of the vertex data
		vk::Format format;

		//Returns how many bytes the format of this input uses
		//TODO: move this utitily somewhere else. -AM
		uint32_t getFormatSize();
	};

	//must be set by parser:
	std::vector<Input> m_input;	 //<-- the index in this vector = order of in-vars in shader.
private:
	
};