#pragma once
#include "SFML/Graphics.hpp"


class  RoundedRect
{
public:
	RoundedRect(float inRad, sf::Vector2f inSize, sf::Vector2f inPos, sf::Color inC) 
	{
		rad = inRad;
		c = inC;
		size = inSize;
		pos = inPos;
		update();
	}	
	void draw(sf::RenderWindow& w) {
		w.draw(v);
	}
	void setSize(sf::Vector2f inSize)
	{
		size = inSize;
		update();
	}
	sf::Vector2f getSize() { return size; }

	sf::VertexArray v;
private:
	void update()
	{
		v.clear();
		v.setPrimitiveType(sf::TriangleFan);
		v.append(sf::Vertex(sf::Vector2f(size.x / 2, size.y / 2) + pos, c));
		for (int i = 0; i < 41; i++)
		{
			float ang = 3.1415 * 2 * i / 40;
			sf::Vector2f p(rad + pos.x + cos(ang) * rad, rad + pos.y + sin(ang) * rad);

			if (i < 10 || i == 40) {
				p.x += size.x - 2 * rad;
				p.y += size.y - 2 * rad;
			}
			else if (i < 20) {
				p.y += size.y - 2 * rad;
			}
			else if (i >= 30 && i < 40) {
				p.x += size.x - 2 * rad;
			}
			v.append(sf::Vertex(p, c));
		}
	}

	float rad;
	sf::Color c;
	sf::Vector2f size;
	sf::Vector2f pos;
};