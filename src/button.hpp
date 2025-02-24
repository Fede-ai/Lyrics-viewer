#include <SFML/Graphics.hpp>

struct Button {
public:
	~Button() {
		if (sprite != nullptr)
			delete sprite;
	}

	sf::Color base, hover, press;
	sf::Sprite* sprite = nullptr;
	sf::Texture texture;

	void loadTexture(std::string path) {
		bool success = texture.loadFromFile(path);

		if (sprite == nullptr)
			sprite = new sf::Sprite(texture);
		else
			sprite->setTexture(texture);
	}
	void setColors(sf::Color b, sf::Color h, sf::Color p) {
		base = b;
		hover = h;
		press = p;

		if (sprite != nullptr)
			sprite->setColor(base);
	}

	//first bool tells you if the mouse is hovering over the button
	//second bool tells you if the hovering state changed (redraw needed)
	std::pair<bool, bool> checkHover(const sf::Event::MouseMoved* e) {
		std::pair<bool, bool> state = { false, false };

		if (sprite->getGlobalBounds().contains(sf::Vector2f(e->position))) {
            if (sprite->getColor() == base) {
                sprite->setColor(hover);
				state.second = true;
            }
			state.first = true;
        }
        else if (sprite->getColor() != base) {
            sprite->setColor(base);
			state.second = true;
        }

		return state;
	}
	//returns true if redraw is needed
	bool mouseLeft() {
		if (sprite->getColor() != base) {
			sprite->setColor(base);
			return true;
		}
		return false;
	}
	//returns true if redraw is needed
	bool mouseDown(const sf::Event::MouseButtonPressed* e) {
		if (sprite->getGlobalBounds().contains(sf::Vector2f(e->position))) {
			sprite->setColor(press);
			return true;
		}
		return false;
	}
	//first bool tells you if the mouse is hovering over the button
	//second bool tells you if the button was clicked
	std::pair<bool, bool> mouseUp(const sf::Event::MouseButtonReleased* e) {
		std::pair<bool, bool> state = { false, false };
		if (sprite->getGlobalBounds().contains(sf::Vector2f(e->position))) {
			state.first = true;
			state.second = sprite->getColor() == press;
			sprite->setColor(hover);
		}
		return state;
	}
};