#pragma once
#include "glm/glm.hpp"

namespace nemu::graphics {

	struct AABB;

	struct Rectangle {

		glm::vec2 position;
		glm::vec2 size;

		Rectangle();
		Rectangle(const AABB& aabb);
		Rectangle(const glm::vec2& position, const glm::vec2& size);
		Rectangle(float x, float y, float width, float height);
		bool Intersects(const Rectangle& other) const;
		bool Contains(const glm::vec2& point) const;
		bool Contains(const glm::vec3& point) const;
		bool operator==(const Rectangle& other) const;
		bool operator!=(const Rectangle& other) const;
		bool operator<(const Rectangle& other) const;
		bool operator>(const Rectangle& other) const;
		glm::vec2 GetMinimumBound() const;
		glm::vec2 GetMaximumBound() const;
	};


}