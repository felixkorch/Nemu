#pragma once
#include "Nemu/Graphics/GLPrimitive.h"
#include "Nemu/Graphics/OpenGL.h"

class IndexBuffer : public GLPrimitive {
	unsigned int count;
public:
	IndexBuffer(const unsigned int* data, unsigned int count)
		: GLPrimitive()
		, count(count)
	{
		Load(data, count);
	}

	IndexBuffer()
		: GLPrimitive()
		, count(0)
	{}

	~IndexBuffer()
	{
		glDeleteBuffers(1, &handle);
	}

	void Load(const unsigned int* data, unsigned int count)
	{
		this->count = count;
		glGenBuffers(1, &handle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
	}

	void Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
	}

	void Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
};