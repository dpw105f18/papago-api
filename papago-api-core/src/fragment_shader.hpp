#pragma once
#include "shader.hpp"
#include "ishader.hpp"

class FragmentShader : public Shader, public IFragmentShader
{
public:
	FragmentShader(const std::vector<char>& bytecode, const std::string& entryPoint);
private:
	
	friend class RenderPass;
	friend class Parser;
	friend class ShaderProgram;
};