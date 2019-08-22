#pragma once
#include "Nemu/Graphics/OpenGL.h"
#include "Nemu/Graphics/VertexArray.h"
#include "Nemu/Graphics/VertexBuffer.h"
#include "Nemu/Graphics/IndexBuffer.h"

namespace nemu::graphics {

	class Texture2D {
		unsigned int id;
		int width, height;

	public:

		Texture2D(int width, int height)
			: width(width)
			, height(height)
		{
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		~Texture2D()
		{
			glDeleteTextures(1, &id);
		}

		Texture2D(Texture2D& other) = delete;
		Texture2D(Texture2D&& other) = delete;
		Texture2D& operator=(Texture2D& other) = delete;
		Texture2D& operator=(Texture2D&& other) = delete;

		void Bind(unsigned slot) const
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, id);
		}

		void Unbind() const
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void SetData(void* pixels)
		{
			glBindTexture(GL_TEXTURE_2D, id);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA,
				GL_UNSIGNED_BYTE, pixels);
		}

		void SetColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
		{
			std::vector<std::uint8_t> data(width * height * 4);
			int i, j;
			for (i = 0; i < width; i++) {
				for (j = 0; j < height; j++) {
					data[i * height * 4 + j * 4 + 0] = r;
					data[i * height * 4 + j * 4 + 1] = g;
					data[i * height * 4 + j * 4 + 2] = b;
					data[i * height * 4 + j * 4 + 3] = a;
				}
			}
			SetData(data.data());
		}

		unsigned int GetID() { return id; }
		int GetWidth() { return width; }
		int GetHeight() { return height; }

	};

}