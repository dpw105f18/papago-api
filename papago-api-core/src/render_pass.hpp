#pragma once
#include <map>

#include "vulkan\vulkan.hpp"
#include "api_enums.hpp"
#include "shader_program.hpp"
#include "irender_pass.hpp"

class IBufferResource;
class DynamicBuffer;
class IImageResource;
class ISampler;

class FragmentShader;
class VertexShader;
class ImageResource;
class Sampler;

class RenderPass : public IRenderPass
{
public:
	explicit operator vk::RenderPass&();
	RenderPass(const vk::UniqueDevice&, vk::UniqueRenderPass&, const ShaderProgram&, const vk::Extent2D&, DepthStencilFlags);

	//Inherited from IRenderPass
	void bindResource(const std::string& name, IBufferResource&) override;
	void bindResource(const std::string& name, IDynamicBuffer&) override;
	void bindResource(const std::string& name, IImageResource&, ISampler&) override;

	vk::UniqueRenderPass m_vkRenderPass;
	vk::UniquePipeline m_vkGraphicsPipeline;
	vk::UniquePipelineLayout m_vkPipelineLayout;
	vk::UniqueDescriptorPool m_vkDescriptorPool;
	vk::UniqueDescriptorSet m_vkDescriptorSet;
	vk::UniqueDescriptorSetLayout m_vkDescriptorSetLayout;
	const ShaderProgram& m_shaderProgram;
	const vk::UniqueDevice& m_vkDevice;
	DepthStencilFlags m_depthStencilFlags;

	std::map<uint32_t, uint32_t> m_bindingAlignment;

	void setupDescriptorSet(const vk::UniqueDevice&, const VertexShader& vertexShader, const FragmentShader& fragmentShader);
	long RenderPass::getBinding(const std::string& name);
	vk::VertexInputBindingDescription getBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();
};
