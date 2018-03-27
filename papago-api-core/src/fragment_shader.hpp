#pragma once
#include "shader.hpp"

class FragmentShader : public Shader
{
public:
	FragmentShader(const FragmentShader&) = delete;
	FragmentShader(FragmentShader&& shader) noexcept : Shader(std::move(shader)) { }
private:
	FragmentShader(const vk::UniqueDevice & device, const std::string & filePath, const std::string& entryPoint);
	
	friend class Device;
	friend class RenderPass;
};