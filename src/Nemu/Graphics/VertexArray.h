#pragma once
#include "Nemu/Graphics/GLPrimitive.h"
#include "Nemu/Graphics/OpenGL.h"
#include "Nemu/Graphics/VertexBuffer.h"

class VertexArray : public GLPrimitive {
	std::vector<VertexBuffer> buffers;
public:
	VertexArray()
		: GLPrimitive()
	{
		glGenVertexArrays(1, &handle);
	}

	~VertexArray()
	{
		glDeleteVertexArrays(1, &handle);
	}

	void AddBuffer(VertexBuffer& vb, const VertexBufferLayout& layout)
	{
		Bind();
		vb.Bind();
		const auto& elements = layout.GetElements();
		std::size_t offset = 0;
		for (unsigned int i = 0; i < elements.size(); i++) {
			const auto& element = elements[i];
			glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset);
			glEnableVertexAttribArray(i);
			offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
		}

		buffers.push_back(std::move(vb));
		Unbind();
	}

	void Bind() const
	{
		glBindVertexArray(handle);
	}

	void Unbind() const
	{
		glBindVertexArray(0);
	}

};