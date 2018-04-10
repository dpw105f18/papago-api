#pragma once
#include "shader.hpp"

class VertexShader : public Shader
{
public:
	VertexShader() : Shader() {} ;
	
private:
	VertexShader(const vk::UniqueDevice& device, const std::string& filePath, const std::string& entryPoint);

	friend class Device;
	friend class RenderPass;
};