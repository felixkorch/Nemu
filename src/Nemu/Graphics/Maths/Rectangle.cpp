#include "Nemu/Graphics/Maths/Rectangle.h"
#include "Nemu/Graphics/Maths/AABB.h"

namespace nemu::graphics {

	Rectangle::Rectangle()
		: position(glm::vec2()), size(glm::vec2())
	{
	}

	Rectangle::Rectangle(const AABB& aabb)
		: position(glm::vec2(aabb.min)), size(glm::vec2(aabb.GetSize()))
	{
	}

	Rectangle::Rectangle(const glm::vec2& position, const glm::vec2& size)
		: position(position), size(size)
	{
	}

	Rectangle::Rectangle(float x, float y, float width, float height)
		: position(glm::vec2(x, y)), size(glm::vec2(width, height))
	{
	}

	bool Rectangle::Intersects(const Rectangle& other) const
	{
		return (GLMGreaterThan(size, other.position) && GLMLessThan(position, other.size)) ||
			(GLMGreaterThan(position, other.size) && GLMLessThan(size, other.position));
	}

	bool Rectangle::Contains(const glm::vec2& point) const
	{
		return GLMGreaterThan(point, GetMinimumBound()) && GLMLessThan(point, GetMaximumBound());
	}

	bool Rectangle::Contains(const glm::vec3& point) const
	{
		return Contains(glm::vec2(point));
	}

	bool Rectangle::operator==(const Rectangle& other) const
	{
		return position == other.position && size == other.size;
	}

	bool Rectangle::operator!=(const Rectangle& other) const
	{
		return !(*this == other);
	}

	bool Rectangle::operator<(const Rectangle& other) const
	{
		return GLMLessThan(size, other.size);
	}

	bool Rectangle::operator>(const Rectangle& other) const
	{
		return GLMGreaterThan(size, other.size);
	}

	glm::vec2 Rectangle::GetMinimumBound() const { return position - size; }
	glm::vec2 Rectangle::GetMaximumBound() const { return position + size; }

}