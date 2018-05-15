#pragma once
#include <string>
#include <map>
#include "common.hpp"


class IVertexShader;
class IFragmentShader;
class VertexShader; 
class Shader;

class PAPAGO_API Parser
{
public:
	Parser(const std::string& compilerPath);
	std::unique_ptr<IVertexShader> compileVertexShader(const std::string& source, const std::string& entryPoint);
	std::unique_ptr<IFragmentShader> compileFragmentShader(const std::string& source, const std::string& entryPoint);
private:
	/* 
	 * Compiles the source GLSL code into spir-v bytecode and returns that.
	 */
	std::vector<char> compile(const std::string& source, const std::string& shaderType) const;

	void setShaderInput(VertexShader& shader, const std::string& source);
	void setShaderUniforms(Shader& shader, const std::string& source);

	std::string m_compilePath;
};