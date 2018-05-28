#pragma once

namespace polygon
{
	/*
	Creates a vertex buffer and an index buffer for various geometric shapes (static methods).
	Each buffer is allocated on the heap at construction, and deleted at destruction of the Mesh object.
	Element data is of type float and unsigned short for the vertex buffer and index buffer respectively.
	The vertex buffer consists of: float[3] position, float[3] normal.
	The longest side of any geometric shape will always be <= 1; it will fit in a "unit box".
	*/
	class Mesh
	{
	public:

		struct Vertex
		{
			float position[3];
			float normal[3];
		};

		/*
		Creates a cylinder approximation, where the top forms an [n]-sided polygon.
		*/
		static Mesh cylinder(unsigned int n);

		/*
		Creates a sphere approximation, using 8[n]^2-2 squares.
		*/
		static Mesh sphere(unsigned int n);

		/*
		Copies the vertex buffer data to [out_data].
		[count], [offset] and [stride] (all in bytes) is used when iterating over the vertex buffer.
		To only get position data (i.e. no normals), call vertexBuffer(&data, 12U, 0U, 24U).
		The flow is:
			1) Set a pointer [offset] bytes in the vertex buffer.
			2) Copy [count] bytes from the pointer to [out_data].
			3) move pointer [stride] bytes forward (from copy start position).
			4) Repeat untill pointer+[count] >= vertex buffer size.
		*/
		void vertexBuffer(void* out_data, unsigned int count, unsigned int offset = 0U, unsigned int stride = 24U);

		/*
		Copies the index buffer data to [out_data].
		*/
		void indexBuffer(unsigned short* out_data);

		/*
		The number of vertices in the vertex buffer.
		*/
		const unsigned int vertexCount;


		/*
		The number of indices in the index buffer.
		*/
		const unsigned int indexCount;


	private:
		Mesh(float* vbuf, const unsigned int vCount, unsigned short* ibuf, const unsigned int iCount);
		~Mesh();

		Vertex* vBuffer;
		unsigned short* iBuffer;

	};
}