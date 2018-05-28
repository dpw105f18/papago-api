#pragma once
#include <vector>
#include <cmath>

namespace circle_mesh
{
	struct vec3
	{
		float x, y, z;

		vec3 normalized() const;
		vec3 operator /(float scalar) const;
		vec3 operator +(vec3 other) const;
	};

	template<typename TVertex, typename TIndex = uint16_t>
	struct Mesh
	{
		using vertex_type = TVertex;
		using index_type = TIndex;

		std::vector<vertex_type> vertices;
		std::vector<index_type> indices;
	};

	template<size_t NIterations>
	Mesh<vec3> generateCircleMesh();

	template<>
	Mesh<vec3> generateCircleMesh<0>()
	{
		return { {
			{ 0.0f,                  0.0f,                  1.0f },
			{ 0.0f,                  0.0f,                 -1.0f },
			{ -1.0f / std::sqrt(2.0f), -1.0f / std::sqrt(2.0f),  0.0f },
			{ 1.0f / std::sqrt(2.0f), -1.0f / std::sqrt(2.0f),  0.0f },
			{ 1.0f / std::sqrt(2.0f),  1.0f / std::sqrt(2.0f),  0.0f },
			{ -1.0f / std::sqrt(2.0f),  1.0f / std::sqrt(2.0f),  0.0f }
			},{
				0, 3, 4,
				0, 4, 5,
				0, 5, 2,
				0, 2, 3,
				1, 4, 3,
				1, 5, 4,
				1, 2, 5,
				1, 3, 2
			} };
	}

	vec3 vec3::normalized() const
	{
		return *this / (std::sqrt(x*x + y*y + z*z));
	};

	vec3 vec3::operator /(float scalar) const
	{
		return { x / scalar, y / scalar, z / scalar };
	};

	vec3 vec3::operator +(vec3 other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	};

	template<size_t NIterations>
	Mesh<vec3> generateCircleMesh()
	{
		auto mesh = generateCircleMesh<NIterations - 1>();
		auto indices = mesh.indices;
		auto vertices = mesh.vertices;
		auto old_indcies_size = indices.size();
		for (auto i = 0; i < old_indcies_size; i += 3) {
			auto a = (vertices[mesh.indices[i + 0]] + vertices[mesh.indices[i + 1]]) / 2.0f;
			auto b = (vertices[mesh.indices[i + 1]] + vertices[mesh.indices[i + 2]]) / 2.0f;
			auto c = (vertices[mesh.indices[i + 2]] + vertices[mesh.indices[i + 0]]) / 2.0f;
			a = a.normalized();
			b = b.normalized();
			c = c.normalized();
			auto vertex_index = vertices.size();
			vertices.push_back(a);
			vertices.push_back(b);
			vertices.push_back(c);

			indices.push_back(mesh.indices[i + 0]);
			indices.push_back(vertex_index + 0);
			indices.push_back(vertex_index + 2);

			indices.push_back(vertex_index + 0);
			indices.push_back(mesh.indices[i + 1]);
			indices.push_back(vertex_index + 1);

			indices.push_back(vertex_index + 1);
			indices.push_back(mesh.indices[i + 2]);
			indices.push_back(vertex_index + 2);

			indices[i + 0] = vertex_index + 0;
			indices[i + 1] = vertex_index + 1;
			indices[i + 2] = vertex_index + 2;
		}
		return { vertices, indices };
	}
}