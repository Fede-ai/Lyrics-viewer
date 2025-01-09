#pragma once
#include <SFML/Graphics.hpp>
#include "cefapp.hpp"

typedef std::pair<std::string, size_t> Line;

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
	void sendTokenToPlayer() const;
	//on its own thread
	void handleSongChange();
	//on its own thread
	void scrollLyrics();

	CefRefPtr<SimpleApp> app_;
	sf::RenderWindow w_;
	bool waitingAuth_ = true;
	std::string accessToken_ = "";
	std::string refreshToken_ = "";

	std::string currentSong_ = "";
	size_t progress_ = 0;
	bool isPlaying = false;

	std::vector<Line> currentLyrics_ = { {"No Lyrics", 0} };
	size_t currentLine_ = 0;

	sf::Font font_;
};