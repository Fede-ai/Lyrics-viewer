#pragma once
#include <SFML/Graphics.hpp>
#include "cefapp.hpp"

class Overlay {
public:
	Overlay(CefRefPtr<SimpleApp> inApp);
	void run();

	bool waitingAuth = true;

private:
	bool getFirstToken();
	void sendTokenToPlayer();

	CefRefPtr<SimpleApp> app_;
	std::string accessToken_ = "";
	std::string refreshToken_ = "";
};