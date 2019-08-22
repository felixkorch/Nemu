#include "Nemu/Graphics/Maths/AABB.h"
#include "Nemu/Graphics/Maths/Rectangle.h"

namespace nemu::graphics {
	AABB::AABB()
		: min(glm::vec3()), max(glm::vec3())
	{
	}

	AABB::AABB(const Rectangle& rectangle)
		: min(rectangle.GetMinimumBound(), 0), max(rectangle.GetMaximumBound(), 0)
	{
	}

	AABB::AABB(const glm::vec2& min, const glm::vec2& max)
		: min(glm::vec3(min, 0)), max(glm::vec3(max, 0))
	{
	}

	AABB::AABB(const glm::vec3& min, const glm::vec3& max)
		: min(min), max(max)
	{
	}

	AABB::AABB(float x, float y, float width, float height)
		: min(glm::vec3(x, y, 0.0f)), max(glm::vec3(width, height, 0.0f))
	{
	}

	AABB::AABB(float x, float y, float z, float width, float height, float depth)
		: min(glm::vec3(x, y, z)), max(glm::vec3(width, height, depth))
	{
	}

	bool AABB::Intersects(const AABB& other) const
	{
		return (GLMGreaterThan(max, other.min) && GLMLessThan(min, other.max)) ||
			(GLMGreaterThan(min, other.max) && GLMLessThan(max, other.min));
	}

	bool AABB::Contains(const glm::vec2& point) const
	{
		return GLMGreaterThan(glm::vec3(point, 0), min) && GLMLessThan(glm::vec3(point, 0), max);
	}

	bool AABB::Contains(const glm::vec3& point) const
	{
		return GLMGreaterThan(point, min) && GLMLessThan(point, max);
	}

	glm::vec3 AABB::Center() const
	{
		return (max + min) * 0.5f;
	}

	bool AABB::operator==(const AABB& other) const
	{
		return min == other.min && max == other.max;
	}

	bool AABB::operator!=(const AABB& other) const
	{
		return !(*this == other);
	}

	bool AABB::operator<(const AABB& other) const
	{
		return GLMLessThan(max, other.min);
	}

	bool AABB::operator>(const AABB& other) const
	{
		return GLMGreaterThan(min, other.max);
	}

	glm::vec3 AABB::GetSize() const { return glm::vec3(abs(max.x - min.x), abs(max.y - min.y), abs(max.z - min.z)); }
}