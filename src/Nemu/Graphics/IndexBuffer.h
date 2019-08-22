#pragma once
#include "Nemu/Graphics/OpenGL.h"

class IndexBuffer {
	unsigned int count;
	unsigned int id;
public:
	IndexBuffer(const unsigned int* data, unsigned int count)
		: count(count)
	{
		Load(data, count);
	}

	IndexBuffer()
		: count(0)
	{}

	~IndexBuffer()
	{
		glDeleteBuffers(1, &id);
	}

	IndexBuffer& operator=(IndexBuffer& other) = delete;
	IndexBuffer& operator=(IndexBuffer&& other) = delete;
	IndexBuffer(IndexBuffer& other) = delete;
	IndexBuffer(IndexBuffer&& other) = delete;

	void Load(const unsigned int* data, unsigned int count)
	{
		this->count = count;
		glGenBuffers(1, &id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
	}

	void Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	}

	void Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
};