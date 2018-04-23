#pragma once
#include "shader.hpp"
#include "ishader.hpp"

class VertexShader : public Shader, public IVertexShader
{
public:

private:
	VertexShader(const std::vector<char>& bytecode, const std::string& entryPoint);

	friend class RenderPass;
	friend class Parser;
	friend class ShaderProgram;
};