#pragma once
#include "Nemu/Graphics/Texture2D.h"
#include "Nemu/Graphics/Maths/Rectangle.h"
#include "Nemu/Graphics/Maths/AABB.h"
#include "glm/glm.hpp"
#include <vector>
#include <memory>

namespace nemu::graphics {

	struct VertexData {
		glm::vec3 vertex;
		glm::vec4 color;
		glm::vec2 uv;
		float tid;
	};

	class Renderable2D {
	protected:
		Rectangle bounds;
		glm::vec4 color;
		std::vector<glm::vec2> uvs;
		std::shared_ptr<Texture2D> texture;
		bool visible;

	public:

		Renderable2D(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
			: bounds(position, size)
			, color(color)
			, uvs(GetDefaultUVs())
			, texture()
			, visible(true)
		{
		}

		virtual ~Renderable2D() = default;

		const glm::vec2& GetPosition() const { return bounds.position; }
		const glm::vec2& GetSize() const { return bounds.size; }

		const Rectangle& GetBounds() const { return bounds; }
		Rectangle& GetBounds() { return bounds; }

		AABB GetBoundingBox() const { return bounds; }
		const glm::vec4 GetColor() const { return color; }

		std::shared_ptr<Texture2D> GetTexture() const { return texture; }
		const std::vector<glm::vec2>& GetUVs() const { return uvs; }

		bool IsVisible() const { return visible; }

		void SetPosition(const glm::vec2& position) { this->bounds.position = position; }
		void SetSize(const glm::vec2& size) { this->bounds.size = size; }
		void SetColor(const glm::vec4& color) { this->color = color; }
		void SetVisible(bool visible) { this->visible = visible; }

		void FlipVertical()
		{
			std::swap(uvs[0], uvs[3]);
			std::swap(uvs[1], uvs[2]);
		}

		void FlipHorizontal()
		{
			std::swap(uvs[0], uvs[1]);
			std::swap(uvs[2], uvs[3]);
		}

		static std::vector<glm::vec2> GetDefaultUVs()
		{
			return std::vector<glm::vec2>{ glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) };
		}
	};

}
