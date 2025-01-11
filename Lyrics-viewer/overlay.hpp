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
	sf::Mutex mutex_;

	std::string accessToken_ = "";
	std::string refreshToken_ = "";
	bool waitingAuth_ = true;
	bool isRunning = true;
	bool isContracted_ = false;
	bool isLocked = false;

	sf::Vector2i startMousePos_ = sf::Vector2i(-1, -1);
	sf::Vector2i startWinPos_;

	std::string currentSong_ = "No Song Playing";
	size_t progress_ = 0;
	size_t duration_ = 0;
	std::vector<Line> currentLyrics_ = { { "No Lyrics", 0 } };
	size_t currentLine_ = 0;
	bool isPlaying_ = false;

	sf::Font font_;
	sf::Texture closeTexture_;
	sf::Texture lockOpenTexture_;
	sf::Texture lockCloseTexture_;
	sf::Texture volumeTexture_;

	sf::Sprite closeSprite_;
	sf::Sprite lockSprite_;
	sf::Sprite volumeSprite_;

	const sf::Color lightGray = sf::Color(150, 150, 150, 150);
	const sf::Color redClose = sf::Color(230, 30, 30, 150);

	sf::Vector2i wSize_ = sf::Vector2i(0, 0);
	sf::FloatRect titleBar_;
	sf::FloatRect background_;
};