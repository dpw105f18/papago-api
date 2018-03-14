#pragma once
#include <string>
class Shader {
public:
	template<class T>
	static T compileFromFile(const std::string& file);
	Shader() = delete;
	Shader(Shader&) = delete;
	Shader(Shader&&) = delete;
};