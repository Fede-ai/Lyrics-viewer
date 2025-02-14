#include <SFML/Graphics.hpp>

struct Button {
public:
	sf::Color base, hover, press;
	sf::Sprite sprite;
	sf::Texture texture;

	void loadTexture(std::string path) {
		texture.loadFromFile(path);
		sprite.setTexture(texture);
	}
	void setColors(sf::Color b, sf::Color h, sf::Color p) {
		base = b;
		hover = h;
		press = p;

		sprite.setColor(base);
	}

	//first bool tells you if the mouse is hovering over the button
	//second bool tells you if the hovering state changed (redraw needed)
	std::pair<bool, bool> checkHover(sf::Event::MouseMoveEvent e) {
		std::pair<bool, bool> state = { false, false };

        if (sprite.getGlobalBounds().contains(float(e.x), float(e.y))) {
            if (sprite.getColor() == base) {
                sprite.setColor(hover);
				state.second = true;
            }
			state.first = true;
        }
        else if (sprite.getColor() != base) {
            sprite.setColor(base);
			state.second = true;
        }

		return state;
	}
	//returns true if redraw is needed
	bool mouseLeft() {
		if (sprite.getColor() != base) {
			sprite.setColor(base);
			return true;
		}
		return false;
	}
	//returns true if redraw is needed
	bool mouseDown(sf::Event::MouseButtonEvent e) {
		if (sprite.getGlobalBounds().contains(float(e.x), float(e.y))) {
			sprite.setColor(press);
			return true;
		}
		return false;
	}
	//first bool tells you if the mouse is hovering over the button
	//second bool tells you if the button was clicked
	std::pair<bool, bool> mouseUp(sf::Event::MouseButtonEvent e) {
		std::pair<bool, bool> state = { false, false };
		if (sprite.getGlobalBounds().contains(float(e.x), float(e.y))) {
			state.first = true;
			state.second = sprite.getColor() == press;
			sprite.setColor(hover);
		}
		return state;
	}
};