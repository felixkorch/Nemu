#pragma once
#include "Nemu/Graphics/OpenGL.h"
#include <vector>
#include <iostream>

struct VertexBufferElement {
	unsigned int type;
	unsigned int count;
	unsigned int normalized;

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type) {
		case GL_FLOAT:         return 4;
		case GL_UNSIGNED_INT:  return 4;
		case GL_UNSIGNED_BYTE: return 1;
		default:break;
		}
		return 0;
	}
};

class VertexBufferLayout {
private:
	std::vector<VertexBufferElement> elements;
	unsigned int stride;
public:
	VertexBufferLayout()
		:stride(0) {}

	template<typename T>
	void Push(unsigned int count);

	const std::vector<VertexBufferElement> GetElements() const { return elements; }

	unsigned int GetStride() const { return stride; }
};

template<>
inline void VertexBufferLayout::Push<float>(unsigned int count)
{
	elements.push_back({ GL_FLOAT, count, GL_FALSE });
	stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
}

template<>
inline void VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
	elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
	stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
}

template<>
inline void VertexBufferLayout::Push<unsigned char>(unsigned int count)
{
	elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
	stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
}