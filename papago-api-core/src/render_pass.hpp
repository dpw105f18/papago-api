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
	std::map<uint64_t, vk::UniquePipeline> m_vkGraphicsPipelines;
	std::map<uint64_t, vk::UniquePipelineLayout> m_vkPipelineLayouts;
	std::map<uint64_t, vk::UniqueDescriptorPool> m_vkDescriptorPools;

	//a map of cached descriptor sets with a bitmask as key. 
	//The mask has 1 on binding index if the binding is a DynamicBuffer, 0 if it is a BufferResource.
	std::map<uint64_t, vk::UniqueDescriptorSet> m_vkDescriptorSets;
	std::map<uint64_t, vk::UniqueDescriptorSetLayout> m_vkDescriptorSetLayouts;
	const ShaderProgram& m_shaderProgram;
	const vk::UniqueDevice& m_vkDevice;
	vk::Extent2D m_vkExtent;
	DepthStencilFlags m_depthStencilFlags;
	uint64_t m_descriptorSetKeyMask;

	std::map<uint32_t, uint32_t> m_bindingAlignment;

	void setupDescriptorSet(const vk::UniqueDevice&, const VertexShader& vertexShader, const FragmentShader& fragmentShader, uint64_t bindingMask);
	long RenderPass::getBinding(const std::string& name);
	vk::VertexInputBindingDescription getBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();

private:
	void cacheNewPipeline(uint64_t bindingMask);
};
