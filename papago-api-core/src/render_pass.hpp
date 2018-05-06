#pragma once
#include "vulkan\vulkan.hpp"
#include "api_enums.hpp"
#include "shader_program.hpp"
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

	vk::UniqueRenderPass m_vkRenderPass;
	vk::UniquePipeline m_vkGraphicsPipeline;
	vk::UniquePipelineLayout m_vkPipelineLayout;
	vk::UniqueDescriptorPool m_vkDescriptorPool;
	vk::UniqueDescriptorSet m_vkDescriptorSet;
	vk::UniqueDescriptorSetLayout m_vkDescriptorSetLayout;
	const ShaderProgram& m_shaderProgram;
	const vk::UniqueDevice& m_vkDevice;

private:

	void setupDescriptorSet(const vk::UniqueDevice&, const VertexShader& vertexShader, const FragmentShader& fragmentShader);
	vk::VertexInputBindingDescription getBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
};
