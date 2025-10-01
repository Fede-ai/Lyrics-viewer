#pragma once
#include <SFML/Graphics.hpp>
#include <mutex>
#include "button.hpp"

typedef std::pair<std::wstring, int> Line;

class Overlay {
public:
	Overlay(std::string at, std::string rt);
	void run();

private:
	bool handleEvent(std::optional<sf::Event> e);
	void drawOverlay();

	//on its own thread
	void handleSongChange();
	//on its own thread
	void scrollLyrics();
	//on its own thread
	void expandWindow();

	sf::RenderWindow w_;
	std::mutex mutex_;

	std::string accessToken_ = "";
	std::string refreshToken_ = "";
	bool waitingAuth_ = true;
	bool isRunning_ = true;
	bool isContracted_ = false;
	bool isLocked_ = false;
	bool isVolume_ = false;

	sf::Vector2i moveStartMousePos_ = sf::Vector2i(-1, -1);
	sf::Vector2i resizeStartMousePos_ = sf::Vector2i(-1, -1);
	sf::Vector2i startWinPos_, startWinSize_;

	std::string currentType_ = "";
	std::wstring currentSong_ = L"No Song Playing";
	std::vector<std::wstring> currentArtists_;
	std::vector<Line> currentLyrics_ = { { L"No Lyrics", 0 } };
	int currentLine_ = 0;
	int volumePercent_ = 0;
	int progress_ = 0;
	int duration_ = 0;
	bool isPlaying_ = false;

	sf::Cursor defaultCursor_;
	sf::Cursor resizeCursor_;

	sf::Font font_;
	sf::Texture resizeTexture_;

	Button closeBut_, lockBut_, volumeBut_;
	sf::Texture lockCloseTexture_;

	Button prevBut_, playBut_, nextBut_;
	sf::Texture pauseTexture_;

	const sf::Color shadowWhite_ = sf::Color(200, 200, 200);
	const sf::Color pressGray_ = sf::Color(150, 150, 150);

	const sf::Color bgGray_ = sf::Color(50, 50, 50, 200);
	const sf::Color tbGray_ = sf::Color(130, 130, 130, 150);

	const sf::Color mainLineCol_ = sf::Color(255, 255, 255);
	const sf::Color secLineCol_ = sf::Color(220, 220, 220);

	sf::Vector2i wSize_ = sf::Vector2i(0, 0);
	sf::FloatRect titleBar_;
	sf::FloatRect background_;
};