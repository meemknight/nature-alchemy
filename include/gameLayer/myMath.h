#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

inline bool aabb(glm::vec4 b1, glm::vec4 b2)
{
	if (((b1.x - b2.x < b2.z)
		&& b2.x - b1.x < b1.z
		)
		&& ((b1.y - b2.y < b2.w)
		&& b2.y - b1.y < b1.w
		)
		)
	{
		return 1;
	}
	return 0;
}

inline bool pointInRect(glm::vec4 b1, glm::vec2 p)
{
	
	if (
		p.x > b1.x
		&& p.y > b1.y
		&& p.x < b1.x + b1.z
		&& p.y < b1.y + b1.w
		)
	{
		return 1;
	}
	return 0;
}