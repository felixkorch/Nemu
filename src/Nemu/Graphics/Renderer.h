#pragma once
#include "Nemu/Graphics/Renderable2D.h"
#include "Nemu/Graphics/Shader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include <memory>
namespace nemu::graphics {
	class Renderer {
		static constexpr std::size_t MAX_SPRITES = 10000;
		static constexpr std::size_t VERTEX_SIZE = sizeof(VertexData);
		static constexpr std::size_t SPRITE_SIZE = (4 * VERTEX_SIZE);
		static constexpr std::size_t BUFFER_SIZE = SPRITE_SIZE * MAX_SPRITES;
		static constexpr std::size_t INDICES_COUNT = (6 * MAX_SPRITES);
		static constexpr std::size_t MAX_TEXTURES = 32;

		VertexArray vao;
		VertexBuffer vbo;
		IndexBuffer ibo;
		Shader shader;
		std::vector<std::shared_ptr<Texture2D>> textures;
		VertexData* dataBuffer;
		std::size_t indexCount;

		float SubmitTexture(const std::shared_ptr<Texture2D>& texture)
		{
			if (textures.size() == MAX_TEXTURES) {
				std::cout << "Max textures exceeded!\n";
				return textures.size();
			}
			if (texture->GetID() == 0) {
				std::cout << "Invalid texture submitted!\n";
				return textures.size();
			}
			textures.push_back(texture);
			return textures.size();
		}

	public:

		Renderer(int width, int height)
		{
			vbo.InitDynamicBufferUsage(BUFFER_SIZE);

			VertexBufferLayout layout;
			layout.Push<float>(3); // Position
			layout.Push<float>(4); // Color
			layout.Push<float>(2); // UV-Coords (Texture coordinates)
			layout.Push<float>(1); // TID (Texture ID)
			vao.AddBuffer(vbo, layout);

			std::vector<unsigned int> indices(INDICES_COUNT);

			int offset = 0;
			for (int i = 0; i < INDICES_COUNT; i += 6) {
				indices[i + 0] = offset + 0;
				indices[i + 1] = offset + 1;
				indices[i + 2] = offset + 2;
				indices[i + 3] = offset + 2;
				indices[i + 4] = offset + 3;
				indices[i + 5] = offset + 0;

				offset += 4;
			}

			ibo.Load(indices.data(), INDICES_COUNT);

			shader.LoadFromString(ShaderPrograms::renderer2d);
			shader.Bind();
			shader.SetUniformMat4f("MVP_Matrix", glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
		}

		void Begin()
		{
			vao.Bind();
			dataBuffer = (VertexData*)vbo.GetInternalPointer();
		}

		void Submit(const std::shared_ptr<Renderable2D>& renderable)
		{
			const auto bounds = renderable->GetBoundingBox();
			const auto& uvs = renderable->GetUVs();
			auto texture = renderable->GetTexture();

			float textureSlot = 0;
			if (texture)
				textureSlot = SubmitTexture(texture);

			*dataBuffer++ = { bounds.min, renderable->GetColor(), uvs[0], textureSlot };
			*dataBuffer++ = { glm::vec3(bounds.max.x, bounds.min.y, 0), renderable->GetColor(), uvs[1], textureSlot };
			*dataBuffer++ = { bounds.max, renderable->GetColor(), uvs[2], textureSlot };
			*dataBuffer = { glm::vec3(bounds.min.x, bounds.max.y, 0), renderable->GetColor(), uvs[3], textureSlot };

			indexCount += 6;
		}

		void Present()
		{
			shader.Bind();
			for (int i = 0; i < textures.size(); i++)
				textures[i]->Bind(i);

			vao.Bind();
			ibo.Bind();
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
			ibo.Unbind();
			vao.Unbind();

			for (int i = 0; i < textures.size(); i++)
				textures[i]->Unbind();

			textures.clear();
			indexCount = 0;

		}


		void End() const
		{
			vbo.ReleasePointer();
		}

	};
}