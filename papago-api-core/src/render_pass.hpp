#pragma once
#include "standard_header.hpp"
#include "api_enums.hpp"

class VertexShader;
class FragmentShader;

class RenderPass
{
public:

private:
	RenderPass(const vk::UniqueDevice&, VertexShader&, FragmentShader&, const vk::Extent2D&, Format);
	vk::UniqueRenderPass createDummyRenderpass(const vk::UniqueDevice&, Format);
	VertexShader& m_vertexShader;
	FragmentShader& m_fragmentShader;
	vk::UniqueRenderPass m_vkRenderPass;
	vk::UniquePipeline m_vkGraphicsPipeline;
	vk::UniquePipelineLayout m_vkPipelineLayout;

	friend class Device;
};