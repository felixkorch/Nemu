#pragma once
#include "Nemu/Graphics/Shader.h"
#include "Nemu/Graphics/OpenGL.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include <cstdint>
#include <vector>

namespace nemu::graphics {

	class Texture2D {

		struct VertexData {
			glm::vec3 pos;
			glm::vec2 uv;
		};

		constexpr static std::size_t TEXTURE_SIZE = sizeof(VertexData) * 4;

		unsigned int handle;
		int textureWidth, textureHeight;
		int width, height;

		glm::mat4 translation, rotation, scale;
		glm::vec2 uv[4];

		unsigned int vao, vbo, ibo;
		VertexData* GPUPointer;

	public:
		Texture2D(int width, int height)
			: handle(0)
			, textureWidth(width)
			, textureHeight(height)
			, width(width)
			, height(height)
			, translation(1)
			, rotation(1)
			, scale(1)
			, uv()
			, vao(0)
			, vbo(0)
			, ibo(0)
			, GPUPointer(nullptr)
		{
			// Texture
			glGenTextures(1, &handle);
			glBindTexture(GL_TEXTURE_2D, handle);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);

			// Buffers
			constexpr unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

			glGenBuffers(1, &ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, TEXTURE_SIZE, nullptr, GL_DYNAMIC_DRAW);

			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			std::size_t offset = 0;
			std::size_t stride = 3 * 4 + 2 * 4; // 3 + 2 floats
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offset);
			glEnableVertexAttribArray(0);
			offset += 3 * 4;
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offset);
			glEnableVertexAttribArray(1);

			uv[0] = glm::vec2(0, 1);
			uv[1] = glm::vec2(1, 1);
			uv[2] = glm::vec2(1, 0);
			uv[3] = glm::vec2(0, 0);
		}

		Texture2D(Texture2D& other) = delete;
		Texture2D(Texture2D&& other) = delete;
		Texture2D& operator=(Texture2D&& other) = delete;
		Texture2D& operator=(Texture2D& other) = delete;

		~Texture2D()
		{
			glDeleteTextures(1, &handle);
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ibo);
		}

		void Draw(int w, int h, Shader& shader)
		{
			auto mvp = glm::ortho(0.0f, (float)w, 0.0f, (float)h, -1.0f, 1.0f) * translation * rotation * scale;

			glBindVertexArray(vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

			GPUPointer = (VertexData*)glMapBufferRange(GL_ARRAY_BUFFER, 0, TEXTURE_SIZE, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			*GPUPointer++ = { glm::vec3(0, 0, 0),          uv[0] };
			*GPUPointer++ = { glm::vec3(width, 0, 0),      uv[1] };
			*GPUPointer++ = { glm::vec3(width, height, 0), uv[2] };
			*GPUPointer   = { glm::vec3(0, height, 0),     uv[3] };


			glUnmapBuffer(GL_ARRAY_BUFFER);

			// Draw
			shader.Bind();
			shader.SetUniformMat4f("ortho", mvp);
			Bind(0);

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

			// Unbind
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			Unbind();
		}

		void SetData(void* pixels)
		{
			glBindTexture(GL_TEXTURE_2D, handle);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA,
				GL_UNSIGNED_BYTE, pixels);
		}

		void Resize(int width, int height)
		{
			this->width = width;
			this->height = height;
		}

		void RotateX(float degrees)
		{
			rotation = glm::rotate(rotation, glm::radians(degrees), glm::vec3(1, 0, 0));
		}

		void RotateY(float degrees)
		{
			rotation = glm::rotate(rotation, glm::radians(degrees), glm::vec3(0, 1, 0));
		}

		void RotateZ(float degrees)
		{
			rotation = glm::rotate(rotation, glm::radians(degrees), glm::vec3(0, 0, 1));
		}

		void Translate(float x, float y)
		{
			translation = glm::translate(translation, glm::vec3(x, y, 0));
		}

		void FlipVertical()
		{
			std::swap(uv[0], uv[3]);
			std::swap(uv[1], uv[2]);
		}

		void FlipHorizontal()
		{
			std::swap(uv[0], uv[1]);
			std::swap(uv[2], uv[3]);
		}

		void SetColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
		{
			std::vector<std::uint8_t> data(textureWidth * textureHeight * 4);
			int i, j;
			for (i = 0; i < textureWidth; i++) {
				for (j = 0; j < textureHeight; j++) {
					data[i * textureHeight * 4 + j * 4 + 0] = r;
					data[i * textureHeight * 4 + j * 4 + 1] = g;
					data[i * textureHeight * 4 + j * 4 + 2] = b;
					data[i * textureHeight * 4 + j * 4 + 3] = a;
				}
			}
			SetData(data.data());
		}

	private:

		void Bind(unsigned slot) const
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, handle);
		}

		void Unbind() const
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

	};

}