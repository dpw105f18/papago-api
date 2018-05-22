#pragma once
#include "iparameter_block.hpp"
#include <vector>
#include "vulkan\vulkan.hpp"

class RenderPass;
class BufferResource;
class DynamicBufferResource;
class ImageResource;
class Sampler;

class ParameterBlock : public IParameterBlock {
public:
	ParameterBlock(const vk::UniqueDevice& device, RenderPass& renderPass, std::vector<ParameterBinding>& bindings);
	vk::UniqueDescriptorPool m_vkPool;
	vk::UniqueDescriptorSet m_vkDescriptorSet;
	uint64_t m_mask;
	RenderPass& m_renderPass;
private:
	void makeVkDescriptorSet(const vk::UniqueDevice& device, std::vector<ParameterBinding>& bindings);
	void bindResources(const vk::UniqueDevice& device, std::vector<ParameterBinding>& bindings);

	vk::WriteDescriptorSet createWriteDescriptorSet(const vk::UniqueDevice& device, const std::string& name, BufferResource& buffer);
	vk::WriteDescriptorSet createWriteDescriptorSet(const vk::UniqueDevice& device, const std::string& name, DynamicBufferResource& buffer);
	vk::WriteDescriptorSet createWriteDescriptorSet(const vk::UniqueDevice& device, const std::string& name, ImageResource& image, Sampler& sampler);
};
	