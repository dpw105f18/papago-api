#pragma once
#include "shader.hpp"

class VertexShader : public Shader
{
public:

private:
	VertexShader(const std::string& filePath, const std::string& entryPoint);

	friend class RenderPass;
	friend class Parser;
	friend class ShaderProgram;
};