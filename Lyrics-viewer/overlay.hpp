#pragma once
#include <SFML/Graphics.hpp>
#include "button.hpp"
#include "cefapp.hpp"

typedef std::pair<std::wstring, int> Line;

class Overlay {
public:
	Overlay(CefRefPtr<SimpleApp> inApp);
	void run();
	bool isWaitingAuth() const {
		return waitingAuth_;
	}

private:
	bool handleEvent(sf::Event e);
	void drawOverlay();

	//blocking function that waits for auth
	bool getFirstToken();

	//on its own thread
	void handleSongChange();
	//on its own thread
	void scrollLyrics();
	//on its own thread
	void expandWindow();

	CefRefPtr<SimpleApp> app_;
	sf::RenderWindow w_;
	sf::Mutex mutex_;

	std::string accessToken_ = "";
	std::string refreshToken_ = "";
	bool waitingAuth_ = true;
	bool isRunning_ = true;
	bool isContracted_ = false;
	bool isLocked_ = false;

	sf::Vector2i startMousePos_ = sf::Vector2i(-1, -1);
	sf::Vector2i startWinPos_;

	std::wstring currentSong_ = L"No Song Playing";
	std::vector<std::wstring> currentArtists_;
	int progress_ = 0;
	int duration_ = 0;
	std::vector<Line> currentLyrics_ = { { L"No Lyrics", 0 } };
	int currentLine_ = 0;
	bool isPlaying_ = false;

	sf::Font font_;

	Button closeBut_, lockBut_, volumeBut_;
	sf::Texture lockCloseTexture_;

	Button prevBut_, playBut_, nextBut_;
	sf::Texture pauseTexture_;

	const sf::Color shadowWhite_ = sf::Color(200, 200, 200);
	const sf::Color pressGray_ = sf::Color(160, 160, 160);

	const sf::Color bgCol_ = sf::Color(50, 50, 50, 200);
	const sf::Color lightGray_ = sf::Color(130, 130, 130, 150);

	const sf::Color mainLineCol_ = sf::Color(255, 255, 255);
	const sf::Color secLineCol_ = sf::Color(220, 220, 220);

	sf::Vector2i wSize_ = sf::Vector2i(0, 0);
	sf::FloatRect titleBar_;
	sf::FloatRect background_;
};