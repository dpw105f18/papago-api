#pragma once
#include "standard_header.hpp"
#include "api_enums.hpp"
#include "shader_program.h"

class FragmentShader;
class VertexShader;

class RenderPass
{
public:
	explicit operator vk::RenderPass&();

private:
	RenderPass(const vk::UniqueDevice&, const ShaderProgram&, const vk::Extent2D&, Format);
	vk::UniqueRenderPass createDummyRenderpass(const vk::UniqueDevice&, Format);
	vk::UniqueRenderPass m_vkRenderPass;
	vk::UniquePipeline m_vkGraphicsPipeline;
	vk::UniquePipelineLayout m_vkPipelineLayout;
	vk::UniqueDescriptorPool m_vkDescriptorPool;
	vk::UniqueDescriptorSet m_vkDescriptorSet;
	vk::UniqueDescriptorSetLayout m_vkDescriptorSetLayout;

	void setupDescriptorSet(const vk::UniqueDevice&, const VertexShader& vertexShader, const FragmentShader& fragmentShader);

	friend class Device;
	friend class CommandBuffer;
};
