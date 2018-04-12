#pragma once
#include <string>
#include <map>
class VertexShader;
class FragmentShader;

class Parser
{
public:
	Parser(const std::string& compilerPath);
	VertexShader compileVertexShader(const std::string& filePath, const std::string& entryPoint);
	FragmentShader compileFragmentShader(const std::string& filePath, const std::string& entryPoint);
private:
	/* 
	 * Compiles [filePath] (a .vert or .frag shader file) into a .spv file (keeping the file name).
	 *	Returns the .spv file (with path) 
	 */
	std::string compile(const std::string& filePath);

	std::string m_compilePath;
};