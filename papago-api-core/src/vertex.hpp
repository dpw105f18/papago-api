#pragma once

struct vec2
{
	float x, y;
};

struct vec3
{
	float x, y, z;
};

struct Vertex
{
	vec3 m_position;
	vec2 m_uv;

	
	static vk::VertexInputBindingDescription getBindingDescription() {
		vk::VertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex; //VK_VERTEX_INPUT_RATE_VERTEX

		return bindingDescription;
	}

	static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].setBinding(0)
			.setLocation(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setOffset(offsetof(Vertex, m_position));

		attributeDescriptions[1].setBinding(0)
			.setLocation(1)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setOffset(offsetof(Vertex, m_uv));

		return attributeDescriptions;
	}
};
