#pragma once
#include "shader.hpp"

class VertexShader : public Shader
{
public:
	
private:
	VertexShader(const vk::UniqueDevice& device, const std::string& filePath, const std::string& entryPoint);

	friend class Device;
};