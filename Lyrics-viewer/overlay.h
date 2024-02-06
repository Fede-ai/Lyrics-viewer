#pragma once
#include <SFML/Graphics.hpp>
#include "roundedrect.h"

class Overlay
{
public:
	Overlay();
	int run();

private:
	void draw();

	sf::RenderWindow w;
	RoundedRect back;
};