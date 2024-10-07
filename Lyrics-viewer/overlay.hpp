#pragma once
#include <SFML/Graphics.hpp>
#include "cefapp.hpp"

class Overlay {
public:
	Overlay(CefRefPtr<SimpleApp> inApp);
	void run();

private:
	void getFirstToken();
	void sendTokenToPlayer();

	CefRefPtr<SimpleApp> app_;
	std::string accessToken_ = "";
	std::string refreshToken_ = "";
};