#include "roundedrect.h"

RoundedRect::RoundedRect(sf::Vector2f inSize, int radius)
	:
	size(inSize),
	rad(radius)
{
	update();
}

std::size_t RoundedRect::getPointCount() const
{
	return 40;
}

sf::Vector2f RoundedRect::getPoint(std::size_t i) const
{
	sf::Vector2f pos(rad, rad);
	float ang = 3.1415 * 2 * i / 40;
	pos.x += cos(ang) * rad;
	pos.y += sin(ang) * rad;

	if (i < 10) {
		pos.x += size.x - 2 * rad;
		pos.y += size.y - 2 * rad;
	}
	else if (i < 20) {
		pos.y += size.y - 2 * rad;
	}
	else if (i < 30) {
	}
	else {
		pos.x += size.x - 2 * rad;
	}

	return pos;
}
