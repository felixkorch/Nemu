#pragma once
#include "Nemu/Graphics/OpenGL.h"
#include "Nemu/Graphics/VertexBuffer.h"

class VertexArray {
	unsigned int id;
public:
	VertexArray()
	{
		glGenVertexArrays(1, &id);
	}

	~VertexArray()
	{
		glDeleteVertexArrays(1, &id);
	}

	VertexArray& operator=(VertexArray& other) = delete;
	VertexArray& operator=(VertexArray&& other) = delete;
	VertexArray(VertexArray& other) = delete;
	VertexArray(VertexArray&& other) = delete;

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

		Unbind();
	}

	void Bind() const
	{
		glBindVertexArray(id);
	}

	void Unbind() const
	{
		glBindVertexArray(0);
	}

};