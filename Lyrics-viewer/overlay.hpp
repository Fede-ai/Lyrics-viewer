#pragma once
#include <SFML/Graphics.hpp>
#include "cefapp.hpp"

typedef std::pair<std::string, int> Line;

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
	std::vector<std::string> currentArtists_;
	int progress_ = 0;
	int duration_ = 0;
	std::vector<Line> currentLyrics_ = { { "No Lyrics", 0 } };
	int currentLine_ = 0;
	bool isPlaying_ = false;

	sf::Font font_;
	sf::Texture closeTexture_;
	sf::Texture lockOpenTexture_;
	sf::Texture lockCloseTexture_;
	sf::Texture volumeTexture_;
	sf::Texture	prevTexture_;
	sf::Texture playTexture_;
	sf::Texture pauseTexture_;

	sf::Sprite closeSprite_;
	sf::Sprite lockSprite_;
	sf::Sprite volumeSprite_;
	sf::Sprite prevSprite_;
	sf::Sprite playSprite_;
	sf::Sprite nextSprite_;

	const sf::Color shadowWhite_ = sf::Color(200, 200, 200);
	const sf::Color pressGray_ = sf::Color(160, 160, 160);

	const sf::Color bgCol_ = sf::Color(40, 40, 40, 210);
	const sf::Color lightGray_ = sf::Color(130, 130, 130, 140);
	const sf::Color darkGray_ = sf::Color(100, 100, 100, 140);

	const sf::Color redClose_ = sf::Color(230, 30, 30, 140);
	const sf::Color mainLineCol_ = sf::Color(255, 255, 255);
	const sf::Color secLineCol_ = sf::Color(210, 210, 210);

	sf::Vector2i wSize_ = sf::Vector2i(0, 0);
	sf::FloatRect titleBar_;
	sf::FloatRect background_;
};