#include "mesh.hpp"
#include <memory>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include "external\glm\glm.hpp"
#include "external\glm\gtx\transform.hpp"

namespace polygon
{
	Mesh Mesh::cylinder(unsigned int n)
	{
		std::vector<Vertex> vertices;
		auto polygonSides = 45;
		auto cakeSlizeRad = glm::radians(360.0f / polygonSides);

		auto polySideLength = std::sinf(cakeSlizeRad / 2.0f);
		auto polyHalfLength = polySideLength / 2.0f;
		auto polyZdist = std::sqrtf(std::pow(polyHalfLength / std::sin(cakeSlizeRad / 2.0f), 2) - std::pow(polyHalfLength, 2));

		std::vector<Vertex> cakeSlizeVertices{
			//top
			{ { 0.0f,			 -polyHalfLength, 0.0f },{ 0.0f, -1.0f, 0.0f } },	//0
			{ { -polyHalfLength, -polyHalfLength, polyZdist },{ 0.0f, -1.0f, 0.0f } },	//1
			{ { polyHalfLength, -polyHalfLength, polyZdist },{ 0.0f, -1.0f, 0.0f } },	//2

			//side
			{ { -polyHalfLength, -polyHalfLength, polyZdist },{ 0.0f,  0.0f, 1.0f } },	//3
			{ { -polyHalfLength,  polyHalfLength, polyZdist },{ 0.0f,  0.0f, 1.0f } },	//4 
			{ { polyHalfLength,  polyHalfLength, polyZdist },{ 0.0f,  0.0f, 1.0f } },	//5 
			{ { polyHalfLength, -polyHalfLength, polyZdist },{ 0.0f,  0.0f, 1.0f } },	//6 

			//bottom											    
			{ { 0.0f,  polyHalfLength, 0.0f },	{ 0.0f,  1.0f, 0.0f } },					//7
			{ { polyHalfLength,  polyHalfLength, polyZdist },{ 0.0f,  1.0f, 0.0f } },	//8
			{ { -polyHalfLength,  polyHalfLength, polyZdist },{ 0.0f,  1.0f, 0.0f } },	//9
		};


		for (auto rotations = 0; rotations < polygonSides; ++rotations) {
			auto rotMat = glm::rotate(glm::radians((360.0f / polygonSides) * rotations), glm::vec3{ 0.0f, 1.0f, 0.0f });
			for (auto& vert : cakeSlizeVertices) {
				auto rotatedPos = glm::vec3(glm::vec4( *(glm::vec3*)vert.position , 0.0f) * rotMat);
				auto rotatedNorm = glm::vec3(glm::vec4( *(glm::vec3*)vert.normal, 0.0f) * rotMat);
				vertices.push_back({ *(float*)&rotatedPos, *(float*)&rotatedNorm });
			}
		}

		std::vector<uint16_t> indices;


		std::vector<uint16_t> sideIndices{
			0, 1, 2,
			3, 4, 5,
			3, 5, 6,
			7, 8, 9
		};

		for (auto side = 0; side < polygonSides; ++side)
		{
			for (auto i : sideIndices) {
				indices.push_back(i + cakeSlizeVertices.size() * side);
			}
		}

		return Mesh((float*)vertices.data(), vertices.size(), indices.data(), indices.size());
	}

	Mesh Mesh::sphere(unsigned int n)
	{
		float angle = glm::radians(360.0f / n);
		float patchSize = std::sinf(angle / 2.0f);

		auto pointCount = n * n * 8;
		auto points = std::vector<glm::vec3>(pointCount);

		//add points in a spiral from origin
		glm::vec3 directions[4] = {
			{ 0.0f, 1.0f, 0.0f	},
			{ 1.0f, 0.0f, 0.0f	},
			{ 0.0f, -1.0f, 0.0f },
			{ -1.0f, 0.0f, 0.0f },
		};

		points.emplace_back( 0.0f, 0.0f, 1.0f );
		auto nextCornerIndex = 0;
		auto corners = 0;
		for (auto i = 0; i < pointCount - 1; ++i) {
			if (points.size() == nextCornerIndex) {
				corners++;
				nextCornerIndex = points.size() + (corners / 2) * 2;
			}

			//go the required direction:


		}
	}

	void Mesh::vertexBuffer(void * out_data, unsigned int count, unsigned int offset, unsigned int stride)
	{
		if (stride == 24U) {
			memcpy(out_data, vBuffer + offset, vertexCount * sizeof(float) - offset);
		}
		else {

			for (auto i = offset; i + count < vertexCount * sizeof(float); i += stride)
			{
				memcpy(out_data, vBuffer, count);
			}
		}
	}

	void Mesh::indexBuffer(unsigned short * out_data)
	{
		memcpy(out_data, iBuffer, indexCount);
	}

	Mesh::Mesh(float * vbuf, const unsigned int vCount, unsigned short * ibuf, const unsigned int iCount)
		: vertexCount(vCount), indexCount(iCount)
	{
		vBuffer = new Vertex[vCount];
		memcpy(vBuffer, vbuf, vCount * 6 * sizeof(float));

		iBuffer = new unsigned short[iCount];
		memcpy(iBuffer, ibuf, iCount * sizeof(unsigned short));
	}

	Mesh::~Mesh()
	{
		for (auto i = 0; i < vertexCount; i += sizeof(Vertex)) {
			Vertex* p = vBuffer + i;
			delete[] p->position;
			delete[] p->normal;
			delete p;
		}

		for (auto i = 0; i < indexCount; i += sizeof(unsigned short)) {
			unsigned short* p = iBuffer + i;
			delete p;
		}
	}
}

#undef GLM_ENABLE_EXPERIMENTAL