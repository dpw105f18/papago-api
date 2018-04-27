#pragma once
#include "api_enums.hpp"
#include "shader_program.h"
#include "irender_pass.hpp"

class FragmentShader;
class VertexShader;
class ImageResource;
class Sampler;

class RenderPass : public IRenderPass
{
public:
	explicit operator vk::RenderPass&();
	RenderPass(const vk::UniqueDevice&, vk::UniqueRenderPass&, const ShaderProgram&, const vk::Extent2D&);

private:
	vk::UniqueRenderPass m_vkRenderPass;
	vk::UniquePipeline m_vkGraphicsPipeline;
	vk::UniquePipelineLayout m_vkPipelineLayout;
	vk::UniqueDescriptorPool m_vkDescriptorPool;
	vk::UniqueDescriptorSet m_vkDescriptorSet;
	vk::UniqueDescriptorSetLayout m_vkDescriptorSetLayout;
	const vk::UniqueDevice& m_vkDevice;

	const ShaderProgram& m_shaderProgram;

	void setupDescriptorSet(const vk::UniqueDevice&, const VertexShader& vertexShader, const FragmentShader& fragmentShader);
	vk::VertexInputBindingDescription getBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();

	friend class Device;
	friend class CommandBuffer;
	friend class ImageResource;
};
