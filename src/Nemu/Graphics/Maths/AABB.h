#pragma once
#include "glm/glm.hpp"

namespace nemu::graphics {

	template <class Vec>
	inline bool GLMLessThan(Vec vec, Vec other)
	{
		return vec.x > other.x && vec.y > other.y;
	}

	template <class Vec>
	inline bool GLMGreaterThan(Vec vec, Vec other)
	{
		return vec.x < other.x && vec.y < other.y;
	}

	struct Rectangle;

	struct AABB {

		glm::vec3 min, max;

		AABB();
		AABB(const Rectangle& rectangle);
		AABB(const glm::vec2& min, const glm::vec2& max);
		AABB(const glm::vec3& min, const glm::vec3& max);
		AABB(float x, float y, float width, float height);
		AABB(float x, float y, float z, float width, float height, float depth);
		bool Intersects(const AABB& other) const;
		bool Contains(const glm::vec2& point) const;
		bool Contains(const glm::vec3& point) const;
		glm::vec3 Center() const;
		bool operator==(const AABB& other) const;
		bool operator!=(const AABB& other) const;
		bool operator<(const AABB& other) const;
		bool operator>(const AABB& other) const;
		glm::vec3 GetSize() const;
	};

}