#pragma once
#include "standard_header.hpp"
#include "api_enums.hpp"

class FragmentShader;
class VertexShader;

class RenderPass
{
public:

private:
	RenderPass(const vk::UniqueDevice&, const VertexShader&, const FragmentShader&, const vk::Extent2D&, Format);
	vk::UniqueRenderPass createDummyRenderpass(const vk::UniqueDevice&, Format);
	const VertexShader& m_vertexShader;
	const FragmentShader& m_fragmentShader;
	vk::UniqueRenderPass m_vkRenderPass;
	vk::UniquePipeline m_vkGraphicsPipeline;
	vk::UniquePipelineLayout m_vkPipelineLayout;

	friend class Device;
};
