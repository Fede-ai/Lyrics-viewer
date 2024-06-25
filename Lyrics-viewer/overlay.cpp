#include "overlay.h"
#include <dwmapi.h>
#include <Windows.h>
#include <iostream>

#pragma comment (lib, "dwmapi.lib")

Overlay::Overlay()
	:
	bg(20, sf::Vector2f(wSize), sf::Vector2f(0, 0), sf::Color(100, 100, 100, 220)),
	titleBg(15, sf::Vector2f(380,30), sf::Vector2f(10, 10), sf::Color(50, 50, 50, 180))
{
}

int Overlay::run()
{
	sf::ContextSettings set;
	set.antialiasingLevel = 8;
	w.create(sf::VideoMode(wSize.x, wSize.y), "Lyrics-viewer", sf::Style::None, set);
	w.setView(sf::View(sf::Vector2f(200, 120), sf::Vector2f(400, 240)));
	w.setFramerateLimit(20);
	lastPos = sf::Mouse::getPosition();

	// window without titlebar (with: w.clear())
	//SetWindowLong(w.getSystemHandle(), GWL_STYLE, WS_VISIBLE | WS_CAPTION);
	//MARGINS margins = { 1, 1, 1, 1 };

	// window completely transparent (with: w.clear(sf::Color::Transparent))
	MARGINS margins;
	margins.cxLeftWidth = -1;
	SetWindowLong(w.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);

	DwmExtendFrameIntoClientArea(w.getSystemHandle(), &margins);
	SetWindowPos(w.getSystemHandle(), HWND_TOPMOST, 10, 10, 0, 0, SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS | SWP_NOSIZE);

	while (w.isOpen()) {
		sf::Event e;
		while (w.pollEvent(e)) {
			if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
				if (titleBg.v.getBounds().contains(sf::Vector2f(sf::Mouse::getPosition(w))))
					isMoving = true;
			}
			else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
				isMoving = false;
			else if (e.type == sf::Event::Closed)
				w.close();
		}

		update();

		draw();
	}

	return 0;
}

void Overlay::update()
{
	auto pos = sf::Mouse::getPosition();

	if (isMoving)
		w.setPosition(w.getPosition() + pos - lastPos);
	else {
		if (w.getPosition().x < pos.x && pos.x < w.getPosition().x + int(wSize.x) &&
			w.getPosition().y + 50 < pos.y && pos.y < w.getPosition().y + int(wSize.y)) {
			if (!isHidden) {
				bg.setSize(sf::Vector2f(float(wSize.x), 50));
				draw();
				w.setSize(sf::Vector2u(wSize.x, 50));
				isHidden = true;
			}
		}
		else if (isHidden) {
			w.setSize(sf::Vector2u(wSize.x, wSize.y));
			bg.setSize(sf::Vector2f(float(wSize.x), float(wSize.y)));
			draw();
			isHidden = false;
		}
	}

	lastPos = pos;
}

void Overlay::draw()
{
	w.setView(sf::View(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(w.getSize()))));
	w.clear(sf::Color::Transparent);

	w.draw(bg.v);
	w.draw(titleBg.v);
	
	w.display();
}