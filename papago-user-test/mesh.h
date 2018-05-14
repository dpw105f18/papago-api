#pragma once
#include <vector>
#include <memory>
#include "external\papago\papago.hpp"
#include "external\glm\glm.hpp"

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
};

struct Mesh
{
	static Mesh Cube(IDevice& device)
	{
		auto vertex_buffer = device.createVertexBuffer(std::vector<Vertex>{
			{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f }},
			{ { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f } },
			{ { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },
			{ { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
			{ { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
			{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
			{ { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },
			{ { 0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f } }
		});

		auto index_buffer = device.createIndexBuffer(std::vector<uint16_t>{
			// Front
			0, 1, 2,
				0, 2, 3,
				// Top
				3, 7, 4,
				3, 4, 0,
				// Right
				3, 2, 6,
				3, 6, 7,
				// Back
				7, 6, 5,
				7, 5, 4,
				// Bottom
				1, 5, 6,
				1, 6, 2,
				// Left
				4, 5, 1,
				4, 1, 0
		});

		return { std::move(vertex_buffer), std::move(index_buffer), 48 };
	}

	std::unique_ptr<IBufferResource> vertex_buffer;
	std::unique_ptr<IBufferResource> index_buffer;
	size_t index_count;

	void use(IRecordingSubCommandBuffer& rCommandBuffer) const
	{
		rCommandBuffer.setVertexBuffer(*vertex_buffer);
		rCommandBuffer.setIndexBuffer(*index_buffer);
	}
};