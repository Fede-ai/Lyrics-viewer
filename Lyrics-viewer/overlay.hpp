#pragma once
#include <SFML/Graphics.hpp>
#include "cefapp.hpp"

class Overlay {
public:
	Overlay(CefRefPtr<SimpleApp> inApp);
	void run();
	bool isWaitingAuth() const;

private:
	bool getFirstToken();
	void sendTokenToPlayer() const;

	CefRefPtr<SimpleApp> app_;
	bool waitingAuth_ = true;

	std::string accessToken_ = "";
	std::string refreshToken_ = "";
};