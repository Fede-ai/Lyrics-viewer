#pragma once
#include "SFML/Graphics.hpp"

class RoundedRect : public sf::Shape
{
public:
	RoundedRect(sf::Vector2f inSize, int radius);

    std::size_t getPointCount() const;
    sf::Vector2f getPoint(std::size_t i) const;

private:
    sf::Vector2f size;
    int rad;
};

