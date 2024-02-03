#include "overlay.h"
#include <dwmapi.h>
#include <Windows.h>

int Overlay::run()
{
	w.create(sf::VideoMode(400, 250), "Lyrics-viewer", sf::Style::None);
	w.setFramerateLimit(10);

	//	window without titlebar (with: w.clear())
	//SetWindowLong(w.getSystemHandle(), GWL_STYLE, WS_VISIBLE | WS_CAPTION);
	//MARGINS margins = { 1, 1, 1, 1 };

	//	window completely transparent (with: w.clear(sf::Color::Transparent))
	MARGINS margins;
	margins.cxLeftWidth = -1;
	SetWindowLong(w.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);

	DwmExtendFrameIntoClientArea(w.getSystemHandle(), &margins);
	SetWindowPos(w.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE);

	while (w.isOpen())
	{
		sf::Event e;
		while (w.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				w.close();
		}

		w.clear(sf::Color(0, 200, 200, 100));

		w.display();
	}

	return 0;
}
