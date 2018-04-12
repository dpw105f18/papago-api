#pragma once
#include "shader.hpp"

class FragmentShader : public Shader
{
public:
private:
	FragmentShader(const std::string & filePath, const std::string& entryPoint);
	
	friend class RenderPass;
	friend class Parser;
	friend class ShaderProgram;
};