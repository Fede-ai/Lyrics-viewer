#include "overlay.hpp"
#include "curlwrapper.hpp"
#include <thread>
#include <fstream>
#include <dwmapi.h>
#include <Windows.h>
#include <chrono>

#pragma comment (lib, "dwmapi.lib")

Overlay::Overlay(std::string at, std::string rt)
	:
    defaultCursor_(sf::Cursor::Type::Arrow),
    resizeCursor_(sf::Cursor::Type::SizeTopRight),
	accessToken_(at),
	refreshToken_(rt)
{
    char rawPath[256];
    GetModuleFileNameA(NULL, rawPath, 256);
    std::string path = std::string(rawPath).substr(0, std::string(rawPath).find_last_of('\\'));

    float width = sf::VideoMode::getDesktopMode().size.x / 3.8f;
	wSize_ = sf::Vector2i(int(width), int(width * .6f));
    wSize_.x = std::max(wSize_.x, 350), wSize_.y = std::max(wSize_.y, 200);

    titleBar_ = sf::FloatRect({ 11, 11 }, { wSize_.x - float(10 + 10 + 24), 25 });
    background_ = sf::FloatRect({ 0, 0 }, sf::Vector2f(wSize_));
    bool success = font_.openFromFile(path + "/resources/AveriaSansLibre-Bold.ttf");
    success = resizeTexture_.loadFromFile(path + "/resources/resize.png");

	closeBut_.loadTexture(path + "/resources/close.png");
    closeBut_.setColors(sf::Color::White, shadowWhite_, sf::Color(190, 30, 30));
    closeBut_.sprite->setPosition({ wSize_.x - 26.f, 5.f });

    success = lockCloseTexture_.loadFromFile(path + "/resources/lock_close.png");
    lockBut_.loadTexture(path + "/resources/lock_open.png");
    lockBut_.setColors(sf::Color::White, shadowWhite_, pressGray_);
    lockBut_.sprite->setPosition({ wSize_.x - 25.f, 25.5f });

    volumeBut_.loadTexture(path + "/resources/volume.png");
    volumeBut_.setColors(sf::Color::White, shadowWhite_, pressGray_);
    volumeBut_.sprite->setPosition({ wSize_.x - 26.f, 47.5f });
    
	prevBut_.loadTexture(path + "/resources/prev.png");
    prevBut_.setColors(sf::Color::White, shadowWhite_, pressGray_);
    prevBut_.sprite->setPosition({ titleBar_.position.x + titleBar_.size.x - 15 - 40 - 8,
        titleBar_.position.y + titleBar_.size.y / 2.f - 8 });

    success = pauseTexture_.loadFromFile(path + "/resources/pause.png");
    playBut_.loadTexture(path + "/resources/play.png");
    playBut_.setColors(sf::Color::White, shadowWhite_, pressGray_);
    playBut_.sprite->setPosition({ titleBar_.position.x + titleBar_.size.x - 15 - 20 - 8,
        titleBar_.position.y + titleBar_.size.y / 2.f - 8 });

    nextBut_.loadTexture(path + "/resources/prev.png");
    nextBut_.setColors(sf::Color::White, shadowWhite_, pressGray_);
    nextBut_.sprite->setScale({ -1, 1 });
    nextBut_.sprite->setPosition({ titleBar_.position.x + titleBar_.size.x - 15 + 8,
        titleBar_.position.y + titleBar_.size.y / 2.f - 8 });
}

void Overlay::run()
{
    sf::sleep(sf::seconds(0.2f));

	//create the window
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    w_.create(sf::VideoMode(sf::Vector2u(wSize_)), "Lyrics Viewer", sf::Style::None, sf::State::Windowed, settings);
	w_.setFramerateLimit(60);
	w_.setKeyRepeatEnabled(false);
    bool success = w_.setActive(false);

	//set window style to borderless
    MARGINS margins;
    margins.cxLeftWidth = -1;
    SetWindowLongA(w_.getNativeHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);

	//move window to the top left corner
    DwmExtendFrameIntoClientArea(w_.getNativeHandle(), &margins);
    SetWindowPos(w_.getNativeHandle(), HWND_TOPMOST, sf::VideoMode::getDesktopMode().size.x - 15
        - wSize_.x, 75, 0, 0, SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS | SWP_NOSIZE);

	drawOverlay();
    //run the thread to check for song changes
    std::thread songChangeThread(&Overlay::handleSongChange, this);
    //run the thread to scroll the lyrics
    std::thread scrollThread(&Overlay::scrollLyrics, this);
    //run the thread to expand the window
    std::thread expandThread(&Overlay::expandWindow, this);

	while (w_.isOpen()) {
        std::optional<sf::Event> e = w_.waitEvent();
        if (handleEvent(e)) {
            //std::cout << "event-issued redraw\n";
            drawOverlay(); 
        }
	}

    isRunning_ = false;
    songChangeThread.join();
    scrollThread.join();
	expandThread.join();
}

bool Overlay::handleEvent(std::optional<sf::Event> e)
{
    if (!e.has_value())
        return false;
    else if (e->is<sf::Event::Closed>()) {
        w_.close();
        return true;
    }
    else if (e->is<sf::Event::MouseEntered>()) {
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            moveStartMousePos_ = sf::Vector2i(-1, -1);
            resizeStartMousePos_ = sf::Vector2i(-1, -1);
        }

        //contract window
        if (!isLocked_ && sf::Mouse::getPosition(w_).y > titleBar_.size.y + titleBar_.position.y * 2) {
            isContracted_ = true;
            background_ = sf::FloatRect({ 0, 0 }, { float(wSize_.x), titleBar_.size.y + titleBar_.position.y * 2 });
            drawOverlay();

            w_.setSize(sf::Vector2u(wSize_.x, int(titleBar_.size.y + titleBar_.position.y * 2)));
            w_.setView(sf::View(sf::Vector2f(wSize_.x / 2.f, titleBar_.size.y / 2.f +
                titleBar_.position.y), sf::Vector2f(w_.getSize())));
        }
    }
    else if (e->is<sf::Event::MouseLeft>()) {
        bool needRedraw = false;

        needRedraw = needRedraw || prevBut_.mouseLeft();
        needRedraw = needRedraw || playBut_.mouseLeft();
        needRedraw = needRedraw || nextBut_.mouseLeft();
        needRedraw = needRedraw || closeBut_.mouseLeft();
        needRedraw = needRedraw || lockBut_.mouseLeft();
        needRedraw = needRedraw || volumeBut_.mouseLeft();

        return needRedraw;
    }
    else if (const auto* move = e->getIf<sf::Event::MouseMoved>()) {
        if (move->position.y > (wSize_.y - 30) + move->position.x && !isContracted_)
            w_.setMouseCursor(resizeCursor_);
        else if (resizeStartMousePos_ == sf::Vector2i(-1, -1))
            w_.setMouseCursor(defaultCursor_);

        if (moveStartMousePos_ != sf::Vector2i(-1, -1))
            w_.setPosition(startWinPos_ - moveStartMousePos_ + sf::Mouse::getPosition());
        else if (resizeStartMousePos_ != sf::Vector2i(-1, -1)) {
            int x = std::max(startWinSize_.x + resizeStartMousePos_.x - sf::Mouse::getPosition().x, 350);
            int y = std::max(startWinSize_.y - resizeStartMousePos_.y + sf::Mouse::getPosition().y, 200);
            
            wSize_ = sf::Vector2i(x, y);
            titleBar_ = sf::FloatRect({ 11, 11 }, { wSize_.x - float(10 + 10 + 24), 25 });
            background_ = sf::FloatRect({ 0, 0 }, sf::Vector2f(wSize_));

            closeBut_.sprite->setPosition({ wSize_.x - 26.f, 5.f });
            lockBut_.sprite->setPosition({ wSize_.x - 25.f, 25.5f });
            volumeBut_.sprite->setPosition({ wSize_.x - 26.f, 47.5f });
            prevBut_.sprite->setPosition({ titleBar_.position.x + titleBar_.size.x - 15 - 40 - 8,
                titleBar_.position.y + titleBar_.size.y / 2.f - 8 });
            playBut_.sprite->setPosition({ titleBar_.position.x + titleBar_.size.x - 15 - 20 - 8,
                titleBar_.position.y + titleBar_.size.y / 2.f - 8 });
            nextBut_.sprite->setPosition({ titleBar_.position.x + titleBar_.size.x - 15 + 8,
                titleBar_.position.y + titleBar_.size.y / 2.f - 8 });

            w_.setView(sf::View(sf::Vector2f(wSize_.x / 2.f, wSize_.y / 2.f), sf::Vector2f(float(x), float(y))));
            w_.setPosition(sf::Vector2i(startWinPos_.x + startWinSize_.x - x, startWinPos_.y));
            w_.setSize(sf::Vector2u(x, y));
            return true;
        }

		bool needRedraw = false;
        //contract window
        if (!isLocked_ && !isContracted_ && move->position.y > titleBar_.size.y + titleBar_.position.y * 2) {
            isContracted_ = true;
            background_ = sf::FloatRect({ 0, 0 }, { float(wSize_.x), titleBar_.size.y + titleBar_.position.y * 2 });
            drawOverlay();

            w_.setSize(sf::Vector2u(wSize_.x, int(titleBar_.size.y + titleBar_.position.y * 2)));
            w_.setView(sf::View(sf::Vector2f(wSize_.x / 2.f, titleBar_.size.y / 2.f +
                titleBar_.position.y), sf::Vector2f(w_.getSize())));
        }
        //expand window (only one specific case)
		else if (isContracted_ && move->position.y < titleBar_.size.y + titleBar_.position.y * 2) {
			isContracted_ = false;
            background_ = sf::FloatRect({ 0, 0 }, sf::Vector2f(wSize_));
			w_.setSize(sf::Vector2u(wSize_.x, wSize_.y));
			w_.setView(sf::View(sf::Vector2f(wSize_.x / 2.f, wSize_.y / 2.f), sf::Vector2f(w_.getSize())));
            needRedraw = true;
		}

        auto state = prevBut_.checkHover(move);
        needRedraw = needRedraw || state.second;
        if (state.first)
            return needRedraw;

        state = playBut_.checkHover(move);
        needRedraw = needRedraw || state.second;
        if (state.first)
            return needRedraw;

        state = nextBut_.checkHover(move);
        needRedraw = needRedraw || state.second;
        if (state.first)
            return needRedraw;

        state = closeBut_.checkHover(move);
        needRedraw = needRedraw || state.second;
        if (state.first)
            return needRedraw;

        state = lockBut_.checkHover(move);
        needRedraw = needRedraw || state.second;
        if (state.first)
            return needRedraw;

        state = volumeBut_.checkHover(move);
        needRedraw = needRedraw || state.second;
        if (state.first)
            return needRedraw;

		return needRedraw;
    }
    else if (const auto* press = e->getIf<sf::Event::MouseButtonPressed>()) {
        if (press->button != sf::Mouse::Button::Left)
            return false;

        if (prevBut_.mouseDown(press))
            return true;
        if (playBut_.mouseDown(press))
            return true;
        if (nextBut_.mouseDown(press))
            return true;
        if (closeBut_.mouseDown(press))
            return true;		
        if (lockBut_.mouseDown(press))
            return true;		
        if (volumeBut_.mouseDown(press))
            return true;

        //start moving
        if (titleBar_.contains(sf::Vector2f(press->position))) {
            moveStartMousePos_ = sf::Mouse::getPosition();
            startWinPos_ = w_.getPosition();
            return false;
        }
        //start resizing
        if (press->position.y > (wSize_.y - 30) + press->position.x && !isContracted_) {
            resizeStartMousePos_ = sf::Mouse::getPosition();
            startWinSize_ = sf::Vector2i(w_.getSize());
            startWinPos_ = w_.getPosition();
            return false;
        }
    }
    else if (const auto* release = e->getIf<sf::Event::MouseButtonReleased>()) {
        if (release->button != sf::Mouse::Button::Left)
            return false;
        
        moveStartMousePos_ = sf::Vector2i(-1, -1);
        resizeStartMousePos_ = sf::Vector2i(-1, -1);

        auto state = prevBut_.mouseUp(release);
        if (state.first) {
            if (state.second) {
                Request req(Request::Methods::POST);
                req.headers = { "Authorization: Bearer " + accessToken_ };
                req.url = "https://api.spotify.com/v1/me/player/previous";

                std::thread([req]() {
                    CurlWrapper::send(req);
                    }).detach();
            }

            return state.second;
        }

        state = playBut_.mouseUp(release);
        if (state.first) {
            if (state.second) {
                Request req(Request::Methods::PUT);
                req.headers = { "Authorization: Bearer " + accessToken_ };

                if (isPlaying_)
                    req.url = "https://api.spotify.com/v1/me/player/pause";
                else
                    req.url = "https://api.spotify.com/v1/me/player/play";

                std::thread([req]() {
                    CurlWrapper::send(req);
                    }).detach();
            }

            return state.second;
        }

        state = nextBut_.mouseUp(release);
        if (state.first) {
            if (state.second) {
                Request req(Request::Methods::POST);
                req.headers = { "Authorization: Bearer " + accessToken_ };
                req.url = "https://api.spotify.com/v1/me/player/next";

                std::thread([req]() {
                    CurlWrapper::send(req);
                    }).detach();
            }

            return state.second;
        }

        state = closeBut_.mouseUp(release);
        if (state.first) {
            if (state.second)
                w_.close();

            return state.second;
        }

        state = lockBut_.mouseUp(release);
        if (state.first) {
            if (state.second)
				isLocked_ = !isLocked_;

            return state.second;
        }

        state = volumeBut_.mouseUp(release);
        if (state.first) {
            if (state.second)
                isVolume_ = !isVolume_;

            return state.second;
        }
    }

    return false;
}
void Overlay::drawOverlay()
{
    //std::cout << "redrawing overlay\n";
    const float vc = titleBar_.position.y * 2 + titleBar_.size.y - 10
        + (w_.getSize().y - titleBar_.position.y * 2 - titleBar_.size.y) / 2.f;

    const auto buildRect = [](sf::FloatRect fr, float r, int n, sf::Color c) {
		auto p = fr.position;
		auto s = fr.size;

        sf::VertexArray rect(sf::PrimitiveType::TriangleFan, 1);
        rect[0].position = { p.x + s.x / 2.f, p.y + s.y / 2.f };
        rect[0].color = c;

        for (int i = 0; i < 4 * n + 1; i++) {
            sf::Vertex v({}, c);
            if (i < n || i == n * 4)
                v.position = sf::Vector2f(p.x + s.x - r, p.y + s.y - r);
            else if (i < n * 2)
                v.position = sf::Vector2f(p.x + r, p.y + s.y - r);
            else if (i < n * 3)
                v.position = sf::Vector2f(p.x + r, p.y + r);
            else
                v.position = sf::Vector2f(p.x + s.x - r, p.y + r);

            float ang = 90 * (i % n) / float(n - 1) + 90.f * int(float(i) / n);
            v.position.x += float(r * std::cos(ang * 3.14159 / 180.f));
            v.position.y += float(r * std::sin(ang * 3.14159 / 180.f));
            rect.append(v);
        }

        return rect;
    };

    mutex_.lock();
    bool success = w_.setActive(true);
    w_.clear(sf::Color::Transparent);
    w_.draw(buildRect(background_, 18, 7, bgGray_));

	//draw the lyrics (prev, current, next)
    if (!isContracted_) {
        const int lineDist = 50;

        //draw previous line
        if (currentLine_ - 1 >= 0) {
            bool splitLine = false;
            sf::Text prev1(font_, currentLyrics_[currentLine_ - 1].first, 15);
            prev1.setFillColor(secLineCol_);
            sf::Text prev2(font_, "", 15);
            prev2.setFillColor(secLineCol_);
            //split the line into two if needed
            if (prev1.getGlobalBounds().size.x > titleBar_.size.x * 15 / 20.f) {
                std::wstring l = currentLyrics_[currentLine_ - 1].first;
                const size_t mid = l.size() / 2;;
                for (size_t d = 0; d < mid; d++) {
                    if (mid + d < l.size() && l[mid + d] == ' ') {
                        prev1.setString(l.substr(0, mid + d));
                        prev2.setString(l.substr(mid + d + 1, l.size()));
                        splitLine = true;
                        break;
                    }
                    if (mid - d >= 0 && l[mid - d] == ' ') {
                        prev1.setString(l.substr(0, mid - d));
                        prev2.setString(l.substr(mid - d + 1, l.size()));
                        splitLine = true;
                        break;
                    }
                }
            }

            prev1.setOrigin({ prev1.getGlobalBounds().size.x / 2.f, prev1.getGlobalBounds().size.y / 2.f });
            prev2.setOrigin({ prev2.getGlobalBounds().size.x / 2.f, prev2.getGlobalBounds().size.y / 2.f });

            //draw second line if needed
            if (splitLine) {
                prev1.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc + 2 - lineDist - 9 });
                prev2.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc + 2 - lineDist + 9 });
                w_.draw(prev2);
            }
            else
                prev1.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc + 2 - lineDist });
            w_.draw(prev1);
        }
        //draw next line
        if (currentLine_ + 1 < int(currentLyrics_.size())) {
            bool splitLine = false;
            sf::Text next1(font_, currentLyrics_[currentLine_ + 1].first, 15);
            next1.setFillColor(secLineCol_);
            sf::Text next2(font_, "", 15);
            next2.setFillColor(secLineCol_);
            //split the line into two if needed
            if (next1.getGlobalBounds().size.x > titleBar_.size.x * 15 / 20.f) {
                std::wstring l = currentLyrics_[currentLine_ + 1].first;
                const size_t mid = l.size() / 2;;
                for (size_t d = 0; d < mid; d++) {
                    if (mid + d < l.size() && l[mid + d] == ' ') {
                        next1.setString(l.substr(0, mid + d));
                        next2.setString(l.substr(mid + d + 1, l.size()));
                        splitLine = true;
                        break;
                    }
                    if (mid - d >= 0 && l[mid - d] == ' ') {
                        next1.setString(l.substr(0, mid - d));
                        next2.setString(l.substr(mid - d + 1, l.size()));
                        splitLine = true;
                        break;
                    }
                }
            }

            next1.setOrigin({ next1.getGlobalBounds().size.x / 2.f, next1.getGlobalBounds().size.y / 2.f });
            next2.setOrigin({ next2.getGlobalBounds().size.x / 2.f, next2.getGlobalBounds().size.y / 2.f });

            //draw second line if needed
            if (splitLine) {
                next1.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc + 2 + lineDist - 9 });
                next2.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc + 2 + lineDist + 9 });
                w_.draw(next2);
            }
            else
                next1.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc + 2 + lineDist });
            w_.draw(next1);
        }

        bool splitLine = false;
        sf::Text line1(font_, currentLyrics_[currentLine_].first, 20);
        line1.setFillColor(mainLineCol_);
        sf::Text line2(font_, "", 20);
        line2.setFillColor(mainLineCol_);
        //split the line into two if needed
        if (line1.getGlobalBounds().size.x > titleBar_.size.x) {
            std::wstring l = currentLyrics_[currentLine_].first;
            const size_t mid = l.size() / 2;;
            for (size_t d = 0; d < mid; d++) {
                if (mid + d < l.size() && l[mid + d] == ' ') {
                    line1.setString(l.substr(0, mid + d));
                    line2.setString(l.substr(mid + d + 1, l.size()));
                    splitLine = true;
                    break;
                }
                if (mid - d >= 0 && l[mid - d] == ' ') {
                    line1.setString(l.substr(0, mid - d));
                    line2.setString(l.substr(mid - d + 1, l.size()));
                    splitLine = true;
                    break;
                }
            }
        }

        line1.setOrigin({ line1.getGlobalBounds().size.x / 2.f, line1.getGlobalBounds().size.y / 2.f });
        line2.setOrigin({ line2.getGlobalBounds().size.x / 2.f, line2.getGlobalBounds().size.y / 2.f });

        //draw second line if needed
        if (splitLine) {
            line1.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc - 11 });
            line2.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc + 11 });
            w_.draw(line2);
        }
        else
            line1.setPosition({ titleBar_.position.x + titleBar_.size.x / 2.f, vc });
        w_.draw(line1);
    }

	//draw the title bar
    w_.draw(buildRect(titleBar_, 10, 7, tbGray_));
	//add artists to the title
    std::wstring titleStr = currentSong_;
    for (int i = 0; i < currentArtists_.size(); i++) {
        if (i == 0)
            titleStr += L" - ";
        else
            titleStr += L", ";
    
        titleStr += currentArtists_[i];
    }
    sf::Text title(font_, titleStr, 14);
	//truncate the title if it's too long
    bool isTitleShortened = false;
    while (title.getGlobalBounds().size.x > titleBar_.size.x - 90) {
        titleStr.resize(titleStr.size() - 1);
        if (titleStr[titleStr.size() - 1] == ' ')
            titleStr.resize(titleStr.size() - 1);
        
        title.setString(titleStr);
        isTitleShortened = true;
    }
    if (isTitleShortened)
        title.setString(titleStr + L"...");
    title.setPosition({ titleBar_.position.x + 8, titleBar_.position.y + 3 });
    w_.draw(title);

	//draw the control buttons
    w_.draw(*prevBut_.sprite);
    playBut_.sprite->setTexture(isPlaying_ ? pauseTexture_ : playBut_.texture);
    w_.draw(*playBut_.sprite);
    w_.draw(*nextBut_.sprite);

	w_.draw(*closeBut_.sprite);
    lockBut_.sprite->setTexture(isLocked_ ? lockCloseTexture_ : lockBut_.texture);
    w_.draw(*lockBut_.sprite);

    //draw the progress/volume bar
    if (!isContracted_) {
        w_.draw(*volumeBut_.sprite);

        sf::Sprite resizeSprite(resizeTexture_);
        resizeSprite.setColor(shadowWhite_);
        resizeSprite.setPosition({ 0, wSize_.y - resizeSprite.getLocalBounds().size.y });
        w_.draw(resizeSprite);

        sf::FloatRect barBg({ wSize_.x - 21.f, 71 }, { 6, wSize_.y - 92.f });
        w_.draw(buildRect(barBg, 3, 4, tbGray_));

        float barPercent;
        if (isVolume_)
            barPercent = volumePercent_ / 100.f;
        else
            barPercent = (duration_ == 0) ? 0 : progress_ / float(duration_);

        sf::FloatRect bar({ wSize_.x - 21.f, 71 }, { 6, 6 + (wSize_.y - 98.f) * barPercent });
        w_.draw(buildRect(bar, 3, 4, sf::Color::White));

		std::string barPercentStr;
        if (isVolume_)
            barPercentStr = std::to_string(volumePercent_) + "/100";
        else {
            int minutes = progress_ / 60'000;
            int seconds = (progress_ / 1'000) % 60;
            barPercentStr = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds) + "/";

            minutes = duration_ / 60'000;
            seconds = (duration_ / 1'000) % 60;
            barPercentStr += std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
		}
        sf::Text barText(font_, barPercentStr, 12);
        barText.setFillColor(shadowWhite_);
        barText.setOrigin({ barText.getGlobalBounds().size.x, 0 });
		barText.setPosition({ wSize_.x - 25.f, wSize_.y - 20.f });
		w_.draw(barText);
    }

    w_.display();
    success = w_.setActive(false);
    mutex_.unlock();
}

void Overlay::handleSongChange()
{   
    //replace ' ' with '+'
	const auto format = [](std::string s) {
		for (size_t i = 0; i < s.size(); i++)
			s[i] = (s[i] == ' ') ? '+' : s[i];

		return s;
    };

    const auto toWstring = [](const std::string& s) {
        std::wstring w = L"";
        for (int i = 0; i < s.size(); i++) {
            if (int(s[i]) > 0)
                w += wchar_t(s[i]);
            else {
                unsigned char ubyte1 = (unsigned char)(s[i]);
                unsigned char ubyte2 = (unsigned char)(s[i + 1]);

                wchar_t codePoint = ((ubyte1 & 0x1F) << 6) | (ubyte2 & 0x3F);
                w += codePoint;
                //skip the next byte
                i++;
            }
        }
        return w;
    };

    const float sleepTime = 1.f;
    Request req = Request(Request::Methods::GET);
    req.url = "https://api.spotify.com/v1/me/player/currently-playing";

    while (isRunning_) {
        req.headers = { "Authorization: Bearer " + accessToken_ };
		auto res = CurlWrapper::send(req);

		//error 301 - failed to retrieve 'currently-playing' data
		if (res.error != "") {
            std::cerr << "error 301: failed to retrieve 'currently-playing' data\n";
			sf::sleep(sf::seconds(sleepTime));
			continue;
		}

		//error 302 - no song playing
		if (res.code == 204) {
            std::cerr << "error 302: no song is currently playing\n";

            if (currentSong_ != L"No Song Playing") {
                currentSong_ = L"No Song Playing";
                currentLyrics_ = { { L"No Lyrics", 0 } };
                duration_ = 0, progress_ = 0;
                currentArtists_.clear();
                isPlaying_ = false;
                drawOverlay();
            }
            else if (isPlaying_) {
				isPlaying_ = false;
				drawOverlay();
            }

            sf::sleep(sf::seconds(sleepTime));
			continue;
		}
        //refresh the expired token (potential error 303)
        else if (res.code == 401) {
            std::cout << "spotify api token is expired\n";

            Request rReq = Request(Request::Methods::POST);
            rReq.url = "https://accounts.spotify.com/api/token";
            rReq.headers = { "Content-Type: application/x-www-form-urlencoded" };
            rReq.body = "grant_type=refresh_token&refresh_token=" + 
                refreshToken_ + "&client_id=244ba241897d4c969d1260ad0c844f91";

            auto rRes = CurlWrapper::send(rReq);
            //token refreshed successfully
            if (rRes.code == 200) {
                auto json = rRes.toJson();
                accessToken_ = json["access_token"];
                refreshToken_ = json["refresh_token"];

				std::fstream tokenFile("token.txt", std::ios::out);
                if (tokenFile.is_open())
                    tokenFile << refreshToken_ << "\n";
				tokenFile.close();

                std::cout << "refreshed expired token\n";
                continue;
            }
			//error 303 - failed to refresh spotify token
            else {
                std::cerr << "error 303: failed to refresh spotify token - " << rRes.code << "\n";
                sf::sleep(sf::seconds(sleepTime));
                continue;
            }
        }
        //error 304 - some other error
		else if (res.code != 200) {
			std::cerr << "error 304: failed to retrieve 'currently-playing' data - " << res.code << "\n";
            sf::sleep(sf::seconds(sleepTime));
            continue;
		}

		auto json = res.toJson();
        bool needRedraw = isPlaying_ != json["is_playing"];
        isPlaying_ = json["is_playing"];
        timeLastCheck_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		//redraw if progress has changed
        if (!json["progress_ms"].is_null()) {
            needRedraw = (progress_ != json["progress_ms"]) || needRedraw;
            progress_ = json["progress_ms"];
        }
        //redraw if volume has changed
		if (!json["device"]["volume_percent"].is_null()) {
			needRedraw = (volumePercent_ != json["device"]["volume_percent"]) || needRedraw;
			volumePercent_ = json["device"]["volume_percent"];
		}
        //set default volume to 0
        else {
			needRedraw = (volumePercent_ != 0) || needRedraw;
			volumePercent_ = 0;
        }

        std::string oldType = currentType_;
        currentType_ = json["currently_playing_type"];
		//a content that is not a 'track' started playing
        if (currentType_ != oldType && currentType_ != "track") {
			currentSong_ = std::wstring(currentType_.begin(), currentType_.end()) + L" - No Lyrics";
            currentLyrics_ = { { L"No Lyrics Available For This Type Of Content", 0 } };
            currentArtists_ = { }, duration_ = 0, progress_ = 0, currentLine_ = 0;

            drawOverlay();
            sf::sleep(sf::seconds(sleepTime));
            continue;
        }
        
		//skip if the current type is not 'track' or if the item is null
        if (currentType_ != "track" || json["item"].is_null()) {
            sf::sleep(sf::seconds(sleepTime));
            continue;
        }

        std::wstring name = toWstring(json["item"]["name"]);
        //new song detected
        if (currentSong_ != name) {
            currentSong_ = name;
            duration_ = json["item"]["duration_ms"];
            currentArtists_ = { }, currentLine_ = 0;
            for (const auto& a : json["item"]["artists"])
                currentArtists_.push_back(toWstring(a["name"]));
            
            currentLyrics_ = { { L"Fetching Lyrics...", 0 } };
            drawOverlay();

            //request lyrics from LRCLIB
			Request lReq = Request(Request::Methods::GET);
            lReq.url = "https://lrclib.net/api/get?track_name=" + format(json["item"]["name"]) +
                "&artist_name=" + format(json["item"]["artists"][0]["name"]) + 
                "&duration=" + std::to_string(int(json["item"]["duration_ms"] / 1000));
            std::cout << lReq.url << "\n";
			auto lRes = CurlWrapper::send(lReq);

			//no lyrics found
            if (lRes.code != 200 || lRes.toJson()["syncedLyrics"].is_null())
                currentLyrics_ = { { L"No Lyrics Available", 0 } };
			//process raw lyrics
            else {
                currentLyrics_ = { { L"", 0 } };
                std::wstring body = toWstring(lRes.toJson()["syncedLyrics"]);
                
                std::wstringstream stream(body);
                std::wstring line;
                while (std::getline(stream, line)) {
                    if (line.empty()) 
                        continue;

                    line = line.substr(line.find_first_of('[') + 1, line.size());
                    while (!(line[0] >= '0' && line[0] <= '9') && line[0] != ':')
                        line = line.substr(1, line.size());

                    int time = 60'000 * std::stoi(line.substr(0, line.find_first_of(':')));
					line = line.substr(line.find_first_of(':') + 1, line.size());

					time += 1'000 * std::stoi(line.substr(0, line.find_first_of('.')));
					line = line.substr(line.find_first_of('.') + 1, line.size());

					time += 10 * std::stoi(line.substr(0, line.find_first_of(']')));
					line = line.substr(line.find_first_of(']') + 1, line.size());

                    if (line[0] == ' ')
                        line = line.substr(1, line.size());

                    auto& last = currentLyrics_[currentLyrics_.size() - 1];
                    if (last.first == L"" && time - last.second < 1'500)
                        last = { line, std::max(0, time - 50) };
                    else
					    currentLyrics_.push_back({ line, std::max(0, time - 50) });
                }
            }
        }

        if (needRedraw)
            drawOverlay();

        //sleep before checking again
        sf::sleep(sf::seconds(sleepTime));
    }
}
void Overlay::scrollLyrics()
{
	while (isRunning_) {
        //error 300 - the lyrics are empty
        if (currentLyrics_.empty()) {
            std::cerr << "error 300: the lyrics are empty\n";
			sf::sleep(sf::seconds(1));
			continue;
        }
        //the lyrics are empty
		if (currentLyrics_.size() == 1) {
            if (currentLine_ != 0) {
                currentLine_ = 0;
                drawOverlay();
            }
			sf::sleep(sf::seconds(1));
			continue;
		}

        size_t surplus = 0;
        if (isPlaying_)
            surplus = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - timeLastCheck_;
		int i = 0;
		while (i < currentLyrics_.size() - 1 && currentLyrics_[size_t(i + 1)].second < int(progress_ + surplus))
			i++;

        if (i != currentLine_) {
			currentLine_ = i;
			drawOverlay();
        }

		sf::sleep(sf::milliseconds(100));
    }
}
void Overlay::expandWindow()
{
    while (isRunning_) {
        sf::sleep(sf::milliseconds(100));

		if (!isContracted_)
			continue;

        sf::IntRect rect;
		rect.size = wSize_;

		if (!rect.contains(sf::Mouse::getPosition(w_))) {
			w_.setSize(sf::Vector2u(wSize_.x, wSize_.y));
			w_.setView(sf::View(sf::Vector2f(wSize_.x / 2.f, wSize_.y / 2.f), sf::Vector2f(w_.getSize())));

			isContracted_ = false;
            background_ = sf::FloatRect({ 0, 0 }, sf::Vector2f(wSize_));
			drawOverlay();
		}
    }
}
