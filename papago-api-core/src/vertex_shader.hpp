#pragma once
#include "shader.hpp"

class VertexShader : public Shader
{
public:

private:
	VertexShader(const std::string& filePath, const std::string& entryPoint);

	struct Input
	{
		uint32_t offset;	//<-- offset in bytes from the beginning of the vertex data
		vk::Format format;

		//Returns how many bytes the format of this input uses
		//TODO: move this utitily somewhere else. -AM
		uint32_t getFormatSize()
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
	};
	
	//must be set by parser:
	std::vector<Input> m_input;	 //<-- the index in this vector = order of in-vars in shader.

	friend class RenderPass;
	friend class Parser;
	friend class ShaderProgram;
};