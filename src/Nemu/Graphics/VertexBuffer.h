#pragma once
#include "Nemu/Graphics/GLPrimitive.h"
#include "Nemu/Graphics/VertexBufferLayout.h"
#include "Nemu/Graphics/OpenGL.h"

class VertexBuffer : public GLPrimitive {
	std::size_t bufferSize;
public:

	VertexBuffer()
		: GLPrimitive()
		, bufferSize(0)
	{
		glGenBuffers(1, &handle);
	}

	VertexBuffer(VertexBuffer&& other) = default;

	~VertexBuffer()
	{
		glDeleteBuffers(1, &handle);
	}

	void Bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, handle);
	}

	void InitStaticBufferUsage(const void* data, std::size_t size)
	{
		bufferSize = size;
		glBindBuffer(GL_ARRAY_BUFFER, handle);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	void InitDynamicBufferUsage(std::size_t size)
	{
		bufferSize = size;
		glBindBuffer(GL_ARRAY_BUFFER, handle);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void* GetInternalPointer() const
	{
		return glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	}

	void ReleasePointer() const
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	void BindLayout(const VertexBufferLayout& layout)
	{
		const auto& elements = layout.GetElements();
		std::size_t offset = 0;
		for (unsigned int i = 0; i < elements.size(); i++) {
			const auto& element = elements[i];
			glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset);
			glEnableVertexAttribArray(i);
			offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
		}
	}

	void Unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

};