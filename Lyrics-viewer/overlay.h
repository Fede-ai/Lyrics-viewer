#pragma once
#include "roundedrect.h"

class Overlay
{
public:
	Overlay();
	int run();

private:
	void update();
	void draw();

	sf::RenderWindow w;
	sf::Vector2i lastPos; 
	sf::Vector2u wSize = sf::Vector2u(400, 240);
	RoundedRect bg, titleBg;
	bool isMoving = false, isHidden = false;
};