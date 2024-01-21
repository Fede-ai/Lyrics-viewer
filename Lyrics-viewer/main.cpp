#include <SFML/Graphics.hpp>
#include <dwmapi.h>
#include <Windows.h>

int main()
{
	sf::Vector2i scrSize(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height);
	sf::RenderWindow w(sf::VideoMode(400, 300), "Music", sf::Style::None);
	w.setFramerateLimit(30);

	//	window without titlebar (with: w.clear())
	//SetWindowLong(w.getSystemHandle(), GWL_STYLE, WS_VISIBLE | WS_CAPTION);
	//MARGINS margins = { 1, 1, 1, 1 };

	//	window completely transparent (with: w.clear(sf::Color::Transparent))
	MARGINS margins;
	margins.cxLeftWidth = -1;
	SetWindowLong(w.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);

	DwmExtendFrameIntoClientArea(w.getSystemHandle(), &margins);

	while (w.isOpen())
	{
		sf::Event e;
		while (w.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				w.close();
			else if (e.type == sf::Event::KeyPressed) 
			{
				if (e.key.code == sf::Keyboard::Space)
					SetWindowPos(w.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE);
			}

		}	

		w.clear(sf::Color::Transparent);
		//w.clear();

		sf::RectangleShape r({ 400, 300 });
		w.draw(r);

		w.display();
	}
	return 0;
}