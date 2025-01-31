#include "overlay.hpp"
#include "curlwrapper.hpp"
#include <dwmapi.h>
#include <Windows.h>
#include <thread>

Overlay::Overlay(CefRefPtr<SimpleApp> inApp)
	:
	app_(inApp)
{
    float width = sf::VideoMode::getDesktopMode().width / 3.8f;
	wSize_ = sf::Vector2i(int(width), int(width * .6f));

    titleBar_ = sf::FloatRect(12, 12, wSize_.x - float(12 + 35), 25);
	background_ = sf::FloatRect(0, 0, float(wSize_.x), float(wSize_.y));

    font_.loadFromFile("resources/AveriaSansLibre-Bold.ttf");
	closeTexture_.loadFromFile("resources/close.png");
    lockOpenTexture_.loadFromFile("resources/lock_open.png");
    lockCloseTexture_.loadFromFile("resources/lock_close.png");
    volumeTexture_.loadFromFile("resources/volume.png");
    prevTexture_.loadFromFile("resources/prev.png"); 
    playTexture_.loadFromFile("resources/play.png");
    pauseTexture_.loadFromFile("resources/pause.png");

	closeSprite_.setTexture(closeTexture_);
    closeSprite_.setColor(lightGray_);
    closeSprite_.setPosition(wSize_.x - 27.f, 10.f);

    lockSprite_.setTexture(lockOpenTexture_);
    lockSprite_.setColor(lightGray_);
    lockSprite_.setPosition(wSize_.x - 27.f, 33.f);

    volumeSprite_.setTexture(volumeTexture_);
    volumeSprite_.setColor(lightGray_);
    volumeSprite_.setPosition(wSize_.x - 27.f, 56.f);

    prevSprite_.setTexture(prevTexture_);
    prevSprite_.setPosition(titleBar_.left + titleBar_.width - 13 - 40,
        titleBar_.top + titleBar_.height / 2.f);
    prevSprite_.setOrigin(prevSprite_.getLocalBounds().width / 2.f,
        prevSprite_.getLocalBounds().height / 2.f);

    playSprite_.setTexture(playTexture_);
    playSprite_.setOrigin(playSprite_.getLocalBounds().width / 2.f,
        playSprite_.getLocalBounds().height / 2.f);
    playSprite_.setPosition(titleBar_.left + titleBar_.width - 13 - 20,
        titleBar_.top + titleBar_.height / 2.f);

    nextSprite_.setTexture(prevTexture_);
    nextSprite_.setScale(-1, 1);
    nextSprite_.setPosition(titleBar_.left + titleBar_.width - 13,
        titleBar_.top + titleBar_.height / 2.f);
    nextSprite_.setOrigin(nextSprite_.getLocalBounds().width / 2.f,
        nextSprite_.getLocalBounds().height / 2.f);
}

void Overlay::run()
{
    //if auth failed, close window without launching player
    if (!getFirstToken()) {
        app_->closeAuthWindows(false);
        return;
    }

    //ATTENTION not needed since right now the browser is not playing the music
    //run the thread to send the token to the player
    //std::thread shareThread(&Overlay::sendTokenToPlayer, this);
    //shareThread.detach();

    //close auth and launch player
    sf::sleep(sf::seconds(0.2f));
    app_->closeAuthWindows(true);

    //close player and shutdown cef
	//ATTENTION this is meant to be called when the overlay is closed. since right 
    //ATTENTION now, since the browser is not playing the music, it can be closed asap
    app_->closePlayerBrowser();

	//create the window
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
	w_.create(sf::VideoMode(wSize_.x, wSize_.y), "Lyrics Viewer", sf::Style::None, settings);
	w_.setFramerateLimit(60);
	w_.setKeyRepeatEnabled(false);
    w_.setActive(false);

	//set window style to borderless
    MARGINS margins;
    margins.cxLeftWidth = -1;
    SetWindowLong(w_.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);

	//move window to the top left corner
    DwmExtendFrameIntoClientArea(w_.getSystemHandle(), &margins);
    SetWindowPos(w_.getSystemHandle(), HWND_TOPMOST, 15, 15,
        0, 0, SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS | SWP_NOSIZE);

	drawOverlay();
    //run the thread to check for song changes
    std::thread songChangeThread(&Overlay::handleSongChange, this);
    //run the thread to scroll the lyrics
    std::thread scrollThread(&Overlay::scrollLyrics, this);

	while (w_.isOpen()) {
		sf::Event e;
		if (w_.waitEvent(e)) {
            if (handleEvent(e))
                drawOverlay(); 
		}
	}

    isRunning = false;
    songChangeThread.join();
    scrollThread.join();
}

bool Overlay::handleEvent(sf::Event e)
{
    if (e.type == sf::Event::Closed) {
        w_.close();
        return true;
    }
    else if (e.type == sf::Event::MouseMoved) {
        if (startMousePos_ != sf::Vector2i(-1, -1))
            w_.setPosition(startWinPos_ - startMousePos_ + sf::Mouse::getPosition());

        //make the close button dark gray (if mouse has just entered the sprite)
        if (closeSprite_.getGlobalBounds().contains(float(e.mouseMove.x), float(e.mouseMove.y))) {
            if (closeSprite_.getColor() == lightGray_) {
                closeSprite_.setColor(darkGray_);
                return true;
            }
            return false;
        }
        //make the close button light gray
        else if (closeSprite_.getColor() != lightGray_) {
            closeSprite_.setColor(lightGray_);
            return true;
        }

        //make the prev button shadow white (if mouse has just entered the sprite)
        if (prevSprite_.getGlobalBounds().contains(float(e.mouseMove.x), float(e.mouseMove.y))) {
            if (prevSprite_.getColor() == sf::Color::White) {
                prevSprite_.setColor(shadowWhite_);
                return true;
            }
            return false;
        }
        //make the prev button white
        else if (prevSprite_.getColor() != sf::Color::White) {
            prevSprite_.setColor(sf::Color::White);
            return true;
        }

        //make the play button shadow white (if mouse has just entered the sprite)
        if (playSprite_.getGlobalBounds().contains(float(e.mouseMove.x), float(e.mouseMove.y))) {
            if (playSprite_.getColor() == sf::Color::White) {
                playSprite_.setColor(shadowWhite_);
                return true;
            }
            return false;
        }
        //make the play button white
        else if (playSprite_.getColor() != sf::Color::White) {
            playSprite_.setColor(sf::Color::White);
            return true;
        }

        //make the next button shadow white (if mouse has just entered the sprite)
        if (nextSprite_.getGlobalBounds().contains(float(e.mouseMove.x), float(e.mouseMove.y))) {
            if (nextSprite_.getColor() == sf::Color::White) {
                nextSprite_.setColor(shadowWhite_);
                return true;
            }
            return false;
        }
        //make the next button white
        else if (nextSprite_.getColor() != sf::Color::White) {
            nextSprite_.setColor(sf::Color::White);
            return true;
        }
    }
    else if (e.type == sf::Event::MouseLeft) {
        bool needRedraw = false;

        //make the close button light gray
        if (closeSprite_.getColor() != lightGray_) {
            closeSprite_.setColor(lightGray_);
            needRedraw = true;
        }
        //make the prev button white
        if (prevSprite_.getColor() != sf::Color::White) {
            prevSprite_.setColor(sf::Color::White);
            needRedraw = true;
        }
        //make the play button white
        if (playSprite_.getColor() != sf::Color::White) {
            playSprite_.setColor(sf::Color::White);
            needRedraw = true;
        }
        //make the next button white
        if (nextSprite_.getColor() != sf::Color::White) {
            nextSprite_.setColor(sf::Color::White);
            needRedraw = true;
        }

        return needRedraw;
    }
    else if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        //make the close button red
        if (closeSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            closeSprite_.setColor(redClose_);
            return true;
        }
        //make the prev button press gray
        else if (prevSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            prevSprite_.setColor(pressGray_);
            return true;
        }
        //make the play button press gray
        else if (playSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            playSprite_.setColor(pressGray_);
            return true;
        }
        //make the next button press gray
        else if (nextSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            nextSprite_.setColor(pressGray_);
            return true;
        }
        //start moving
        else if (titleBar_.contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            startMousePos_ = sf::Mouse::getPosition();
            startWinPos_ = w_.getPosition();
            return false;
        }
    }
    else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
        startMousePos_ = sf::Vector2i(-1, -1);

        //close the window
        if (closeSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            if (closeSprite_.getColor() == redClose_) {
                w_.close();
                return true;
            }
            return false;
        }
        //go to prev song
        else if (prevSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            if (prevSprite_.getColor() == pressGray_) {
                prevSprite_.setColor(shadowWhite_);
                
                Request req(Request::Methods::POST);
                req.headers = { "Authorization: Bearer " + accessToken_ };
                req.url = "https://api.spotify.com/v1/me/player/previous";

                std::thread([req]() {
                    CurlWrapper::send(req);
                    }).detach();

                return true;
            }
            return false;
        }
        //pause - play
        else if (playSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            if (playSprite_.getColor() == pressGray_) {
                playSprite_.setColor(shadowWhite_);

                Request req(Request::Methods::PUT);
                req.headers = { "Authorization: Bearer " + accessToken_ };

                if (isPlaying_)
                    req.url = "https://api.spotify.com/v1/me/player/pause";
                else
                    req.url = "https://api.spotify.com/v1/me/player/play";

                std::thread([req]() {
                    CurlWrapper::send(req);
                    }).detach();

                return true;
            }
            return false;
        }
        //go to next song
        else if (nextSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
            if (nextSprite_.getColor() == pressGray_) {
                nextSprite_.setColor(shadowWhite_);

                Request req(Request::Methods::POST);
                req.headers = { "Authorization: Bearer " + accessToken_ };
                req.url = "https://api.spotify.com/v1/me/player/next";

                std::thread([req]() {
                    CurlWrapper::send(req);
                    }).detach();

                return true;
            }
            return false;
        }
    }

    return false;
}
void Overlay::drawOverlay()
{
    std::cout << "redrawing overlay\n";
    const float vc = (titleBar_.top + titleBar_.height + w_.getSize().y) / 2.f - 4;
    const auto buildRect = [](sf::FloatRect fr, float r, int n, sf::Color c) {
		auto p = fr.getPosition();
		auto s = fr.getSize();

        sf::VertexArray rect(sf::TriangleFan, 1);
        rect[0].position = { (p.x + s.x) / 2.f, (p.y + s.y) / 2.f };
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
    w_.setActive(true);
    w_.clear(sf::Color::Transparent);
    w_.draw(buildRect(background_, 20, 7, bgCol_));

    //draw previous line
    if (currentLine_ - 1 >= 0) {
        bool splitLine = false;
        sf::Text prev1(currentLyrics_[currentLine_ - 1].first, font_, 15);
        prev1.setFillColor(secLineCol_);
        sf::Text prev2("", font_, 15);
        prev2.setFillColor(secLineCol_);
        //split the line into two if needed
        if (prev1.getGlobalBounds().width > titleBar_.width * 15 / 20.f) {
            std::string l = currentLyrics_[currentLine_ - 1].first;
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

        prev1.setOrigin(prev1.getGlobalBounds().width / 2.f, prev1.getGlobalBounds().height / 2.f);
        prev2.setOrigin(prev2.getGlobalBounds().width / 2.f, prev2.getGlobalBounds().height / 2.f);

        //draw second line if needed
        if (splitLine) {
            prev1.setPosition(titleBar_.left + titleBar_.width / 2.f, vc - 58);
            prev2.setPosition(titleBar_.left + titleBar_.width / 2.f, vc - 42);
            w_.draw(prev2);
        }
        else
            prev1.setPosition(titleBar_.left + titleBar_.width / 2.f, vc - 50);
        w_.draw(prev1);
    }
    //draw next line
    if (currentLine_ + 1 < currentLyrics_.size()) {
        bool splitLine = false;
        sf::Text next1(currentLyrics_[currentLine_ + 1].first, font_, 15);
        next1.setFillColor(secLineCol_);
        sf::Text next2("", font_, 15);
        next2.setFillColor(secLineCol_);
        //split the line into two if needed
        if (next1.getGlobalBounds().width > titleBar_.width * 15 / 20.f) {
            std::string l = currentLyrics_[currentLine_ + 1].first;
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

        next1.setOrigin(next1.getGlobalBounds().width / 2.f, next1.getGlobalBounds().height / 2.f);
        next2.setOrigin(next2.getGlobalBounds().width / 2.f, next2.getGlobalBounds().height / 2.f);

        //draw second line if needed
        if (splitLine) {
            next1.setPosition(titleBar_.left + titleBar_.width / 2.f, vc + 42 + 2);
            next2.setPosition(titleBar_.left + titleBar_.width / 2.f, vc + 58 + 2);
            w_.draw(next2);
        }
        else
            next1.setPosition(titleBar_.left + titleBar_.width / 2.f, vc + 50 + 2);
        w_.draw(next1);
    }

    w_.draw(buildRect(titleBar_, 10, 7, lightGray_));

    std::string titleStr = currentSong_;
    for (int i = 0; i < currentArtists_.size(); i++) {
        if (i == 0)
            titleStr += " - ";
        else
            titleStr += ", ";

        titleStr += currentArtists_[i];
    }
    sf::Text title(titleStr, font_, 14);
    bool isTitleShortened = false;
    while (title.getGlobalBounds().width > titleBar_.width - 85) {
        titleStr.resize(titleStr.size() - 1);
        if (titleStr[titleStr.size() - 1] == ' ')
            titleStr.resize(titleStr.size() - 1);

        title.setString(titleStr);
        isTitleShortened = true;
    }
    if (isTitleShortened)
        title.setString(titleStr + "...");
    title.setPosition(titleBar_.left + 8, 15);
    w_.draw(title);

    if (isPlaying_)
        playSprite_.setTexture(pauseTexture_);
    else
        playSprite_.setTexture(playTexture_);
    w_.draw(playSprite_);

    w_.draw(prevSprite_);
    w_.draw(nextSprite_);

	w_.draw(closeSprite_);
    w_.draw(lockSprite_);
    w_.draw(volumeSprite_);

    bool splitLine = false;
	sf::Text line1(currentLyrics_[currentLine_].first, font_, 20);
    line1.setFillColor(mainLineCol_);
    sf::Text line2("", font_, 20);
    line2.setFillColor(mainLineCol_);
    //split the line into two if needed
    if (line1.getGlobalBounds().width > titleBar_.width) {
        std::string l = currentLyrics_[currentLine_].first;
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

    line1.setOrigin(line1.getGlobalBounds().width / 2.f, line1.getGlobalBounds().height / 2.f);
    line2.setOrigin(line2.getGlobalBounds().width / 2.f, line2.getGlobalBounds().height / 2.f);

    //draw second line if needed
    if (splitLine) {
        line1.setPosition(titleBar_.left + titleBar_.width / 2.f, vc - 10);
        line2.setPosition(titleBar_.left + titleBar_.width / 2.f, vc + 10);
        w_.draw(line2);
    }
    else
        line1.setPosition(titleBar_.left + titleBar_.width / 2.f, vc);
    w_.draw(line1);

    w_.display();
    w_.setActive(false);
    mutex_.unlock();
}

bool Overlay::getFirstToken()
{
    //create read-only named pipe
    HANDLE hPipe = CreateNamedPipeW(TEXT("\\\\.\\pipe\\firstToken"), PIPE_ACCESS_INBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 0, 512, 0, NULL);

    //failed to create the pipe (error 100)
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "error 100: " << GetLastError() << "\n";
        std::exit(100);
    }

    std::string info = "";
    //wait for a client to connect
    if (ConnectNamedPipe(hPipe, NULL)) {
        char buffer[512] = {};
        DWORD bytesRead;
        if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            buffer[bytesRead] = '\0'; //null terminate the string
            info = buffer;
        }
        //failed to read the message (error 103)
        else {
            std::cerr << "error 103: " << GetLastError() << "\n";
            std::exit(103);
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    waitingAuth_ = false;

    //failed to authenticate spotify (error 200)
    if (info[0] == 'f') {
        std::cerr << "error 200: " << info.substr(1, info.size()) << "\n";
        return false;
    }

    size_t sep = info.find_first_of('+');
    accessToken_ = info.substr(1, sep - 1);
    refreshToken_ = info.substr(sep + 1, info.size());
    return true;
}

void Overlay::sendTokenToPlayer() const
{
    //create write-only named pipe
    HANDLE hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\shareToken"), PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 512, 512, 0, NULL);

    //failed to create the pipe (error 104)
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "error 104: " << GetLastError() << "\n";
        std::exit(100);
    }

    while (true) {
        //wait for a client to connect
        if (ConnectNamedPipe(hPipe, NULL)) {
            const char* msg = accessToken_.c_str();
            DWORD bytesWritten;
            BOOL success = WriteFile(hPipe, msg, (DWORD)strlen(msg), &bytesWritten, NULL);

            //failed to send message through pipe (error 106)
            if (!success) {
                std::cerr << "error 106: " << GetLastError() << "\n";
            }
            
            sf::sleep(sf::seconds(0.2f));
        }
        DisconnectNamedPipe(hPipe);
    }
}
void Overlay::handleSongChange()
{   
    //replace ' ' with '+'
	const auto format = [](std::string s) {
		for (size_t i = 0; i < s.size(); i++)
			s[i] = (s[i] == ' ') ? '+' : s[i];

		return s;
    };

    Request req = Request(Request::Methods::GET);
    req.url = "https://api.spotify.com/v1/me/player/currently-playing";

    while (isRunning) {
        req.headers = { "Authorization: Bearer " + accessToken_ };
		auto res = CurlWrapper::send(req);

        //no song playing
		if (res.code == 204) {
            if (currentSong_ != "No Song Playing") {
                currentSong_ = "No Song Playing";
                currentLyrics_ = { { "No Lyrics", 0 } };
                currentArtists_.clear();
                isPlaying_ = false;
                drawOverlay();
            }
            else if (isPlaying_) {
				isPlaying_ = false;
				drawOverlay();
            }

            sf::sleep(sf::seconds(1));
			continue;
		}
        //refresh the expired token
        else if (res.code == 401) {
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
                std::cout << "refreshed expired token\n";
                continue;
            }
            else {
                std::cout << "ERROR (token refresh): " << rRes.code << "\n";
                sf::sleep(sf::seconds(1));
                continue;
            }
        }
        //some other error
		else if (res.code != 200) {
			std::cout << "ERROR (song change): " << res.error << " - " << res.code << "\n";
            sf::sleep(sf::seconds(1));
            continue;
		}

		auto json = res.toJson();
        bool needRedraw = progress_ != json["progress_ms"] || isPlaying_ != json["is_playing"];
        progress_ = json["progress_ms"];
        isPlaying_ = json["is_playing"];

        //the content playing is not a song
        if (json["currently_playing_type"] != "track") {
            currentSong_ = json["currently_playing_type"];
            currentLyrics_ = { {"No Lyrics For This Type Of Content", 0} };
            currentArtists_ = { }, duration_ = 0, currentLine_ = 0;

            drawOverlay();
            sf::sleep(sf::seconds(1));
            continue;
        }
        //new song detected
        if (json["item"]["name"] != currentSong_) {
		    currentSong_ = json["item"]["name"];
            duration_ = json["item"]["duration_ms"];
            currentArtists_ = { }, currentLine_ = 0;
            for (const auto& a : json["item"]["artists"])
                currentArtists_.push_back(a["name"]);

            //request lyrics from LRCLIB
			Request lReq = Request(Request::Methods::GET);
            lReq.url = "https://lrclib.net/api/get?track_name=" + format(currentSong_) + 
                "&artist_name=" + format(json["item"]["artists"][0]["name"]) + 
                "&duration=" + std::to_string(int(json["item"]["duration_ms"] / 1000));
            std::cout << lReq.url << "\n";
			auto lRes = CurlWrapper::send(lReq);

			//no lyrics found
            if (lRes.code != 200 || lRes.toJson()["syncedLyrics"].is_null())
                currentLyrics_ = { { "No Lyrics", 0 } };
			//process raw lyrics
            else {
                currentLyrics_ = { { "", 0 } };
                std::istringstream stream(std::string(lRes.toJson()["syncedLyrics"]));
                std::string line;
                while (std::getline(stream, line)) {
                    if (line.empty()) 
                        continue;

                    line = line.substr(line.find_first_of('[') + 1, line.size());
					int time = 60'000 * std::stoi(line.substr(0, line.find_first_of(':')));
					line = line.substr(line.find_first_of(':') + 1, line.size());

					time += 1'000 * std::stoi(line.substr(0, line.find_first_of('.')));
					line = line.substr(line.find_first_of('.') + 1, line.size());

					time += 10 * std::stoi(line.substr(0, line.find_first_of(']')));
					line = line.substr(line.find_first_of(']') + 1, line.size());

                    if (line[0] == ' ')
                        line = line.substr(1, line.size());

                    auto& last = currentLyrics_[currentLyrics_.size() - 1];
                    if (last.first == "" && time - last.second < 1'500)
                        last = { line, std::max(0, time - 500) };
                    else
					    currentLyrics_.push_back({ line, std::max(0, time - 500) });
                }
            }
        }

        if (needRedraw)
            drawOverlay();

        //sleep before checking again
        sf::sleep(sf::seconds(1));
    }
}
void Overlay::scrollLyrics()
{
	while (isRunning) {
        //the lyrics are empty
		if (currentLyrics_.size() == 1) {
			currentLine_ = 0;
			sf::sleep(sf::seconds(1));
			continue;
		}

		int i = 0;
		while (i < currentLyrics_.size() && currentLyrics_[i].second < progress_)
			i++;

        if (i == 0) {
			std::cout << "LINE COUNTING ERROR\n";
            std::exit(-1);
        }

        if (i - 1 != currentLine_) {
			currentLine_ = i - 1;
			drawOverlay();
        }

		sf::sleep(sf::milliseconds(100));
    }
}
