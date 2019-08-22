#pragma once
#include "Nemu/Graphics/Renderable2D.h"

namespace nemu::graphics {
	class Sprite : public Renderable2D {
	public:
		Sprite(const std::shared_ptr<Texture2D>& texture)
			: Renderable2D(glm::vec2(), glm::vec2(texture->GetWidth(), texture->GetHeight()), glm::vec4(1))
		{
			this->texture = texture;
		}

		Sprite(float x, float y, const std::shared_ptr<Texture2D>& texture)
			: Renderable2D(glm::vec2(x, y), glm::vec2(texture->GetWidth(), texture->GetHeight()), glm::vec4(1))
		{
			this->texture = texture;
		}

		Sprite(float x, float y, float width, float height, glm::vec4 color)
			: Renderable2D(glm::vec2(x, y), glm::vec2(width, height), color)
		{}

		Sprite(float x, float y, float width, float height, const std::shared_ptr<Texture2D>& texture)
			: Renderable2D(glm::vec2(x, y), glm::vec2(width, height), glm::vec4(1))
		{
			this->texture = texture;
		}

		void SetUV(const std::vector<glm::vec2>& uv)
		{
			this->uvs = uv;
		}

		void SetTexture(const std::shared_ptr<Texture2D>& texture) { this->texture = texture; }
	};
}