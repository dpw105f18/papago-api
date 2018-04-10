#pragma once
#include <string>
#include <map>
class VertexShader;
class FragmentShader;

class Parser
{
public:
	Parser(const std::string& compilerPath);
	VertexShader compileVertexShader(const std::string& filePath);
	//FragmentShader compileFragmentShader(const std::string& filePath);
private:
	std::string m_compilePath;
};