#pragma once
#include <map>

#include "vulkan\vulkan.hpp"
#include "api_enums.hpp"
#include "shader_program.hpp"
#include "irender_pass.hpp"

class IBufferResource;
class DynamicBufferResource;
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


	vk::UniqueRenderPass m_vkRenderPass;

	//The mask has 1 on binding index if the binding is a DynamicBuffer, 0 if it is a BufferResource.
	std::map<uint64_t, vk::UniquePipeline> m_vkGraphicsPipelines;
	std::map<uint64_t, vk::UniquePipelineLayout> m_vkPipelineLayouts;
	std::map<uint64_t, vk::UniqueDescriptorSetLayout> m_vkDescriptorSetLayouts;
	const ShaderProgram& m_shaderProgram;
	const vk::UniqueDevice& m_vkDevice;
	vk::Extent2D m_vkExtent;
	DepthStencilFlags m_depthStencilFlags;

	void setupDescriptorSetLayout(const vk::UniqueDevice&, const VertexShader& vertexShader, const FragmentShader& fragmentShader, uint64_t bindingMask);
	long getBinding(const std::string& name) const;
	vk::VertexInputBindingDescription getBindingDescription();
	std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();

	vk::UniquePipeline& getPipeline(uint64_t mask);
	vk::UniquePipelineLayout& getPipelineLayout(uint64_t mask);
	vk::UniqueDescriptorSetLayout& getDescriptorSetLayout(uint64_t mask);

	void cacheNewPipeline(uint64_t bindingMask);
	void createNewPipelineIfNone(uint64_t mask);
private:
};
