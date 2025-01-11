#include "overlay.hpp"
#include "curlwrapper.hpp"
#include <dwmapi.h>
#include <Windows.h>
#include <thread>

Overlay::Overlay(CefRefPtr<SimpleApp> inApp)
	:
	app_(inApp)
{
    float width = sf::VideoMode::getDesktopMode().width / 4.f;
	wSize_ = sf::Vector2i(int(width), int(width * .6f));

    titleBar_ = sf::FloatRect(12, 12, wSize_.x - float(12 + 35), 25);
	background_ = sf::FloatRect(0, 0, float(wSize_.x), float(wSize_.y));

    font_.loadFromFile("resources/AveriaSansLibre-Bold.ttf");
	closeTexture_.loadFromFile("resources/close.png");
    lockOpenTexture_.loadFromFile("resources/lock_open.png");
    lockCloseTexture_.loadFromFile("resources/lock_close.png");
    volumeTexture_.loadFromFile("resources/volume.png");;

	closeSprite_.setTexture(closeTexture_);
    closeSprite_.setColor(lightGray);
    closeSprite_.setPosition(wSize_.x - 27.f, 10.f);

    lockSprite_.setTexture(lockOpenTexture_);
    lockSprite_.setColor(lightGray);
    lockSprite_.setPosition(wSize_.x - 27.f, 33.f);

    volumeSprite_.setTexture(volumeTexture_);
    volumeSprite_.setColor(lightGray);
    volumeSprite_.setPosition(wSize_.x - 27.f, 56.f);
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
    
    //Request r = Request(Request::Methods::POST);
    //r.url = "https://accounts.spotify.com/api/token";
    //r.headers = { "Content-Type: application/x-www-form-urlencoded" };
    //r.body = "grant_type=refresh_token&refresh_token=" + refreshToken + "&client_id=244ba241897d4c969d1260ad0c844f91";
}

bool Overlay::handleEvent(sf::Event e)
{
    if (e.type == sf::Event::MouseMoved) {
        if (startMousePos_.x != -1)
            w_.setPosition(startWinPos_ - startMousePos_ + sf::Mouse::getPosition());

        //make the close button red
        if (closeSprite_.getGlobalBounds().contains(float(e.mouseMove.x), float(e.mouseMove.y))) {
            if (closeSprite_.getColor() != redClose) {
                closeSprite_.setColor(redClose);
                return true;
            }
        }
		//make the close button gray
        else if (closeSprite_.getColor() != lightGray) {
            closeSprite_.setColor(lightGray);
            return true;
        }
    }
    else if (e.type == sf::Event::MouseLeft) {
	    //make the close button gray
        if (closeSprite_.getColor() != lightGray) {
            closeSprite_.setColor(lightGray);
            return true;
        }
    }
    else if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
		//close the window
        if (closeSprite_.getGlobalBounds().contains(float(e.mouseButton.x), float(e.mouseButton.y))) {
			w_.close();
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
    }

    return false;
}
void Overlay::drawOverlay()
{
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
    std::cout << "redrawing overlay\n";

    w_.draw(buildRect(background_, 20, 7, sf::Color(50, 50, 50, 200)));
    w_.draw(buildRect(titleBar_, 10, 7, lightGray));

    sf::Text title(currentSong_, font_, 14);
    title.setPosition(20, 15);
    w_.draw(title);

	w_.draw(closeSprite_);
    w_.draw(lockSprite_);
    w_.draw(volumeSprite_);

	sf::Text l(currentLyrics_[currentLine_].first, font_, 20);
	l.setFillColor(sf::Color::White);
    l.setOrigin(l.getGlobalBounds().width / 2.f, l.getGlobalBounds().height / 2.f);
	l.setPosition(w_.getSize().x / 2.f, w_.getSize().y / 2.f);
	w_.draw(l);

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
            currentSong_ = "No Song";
            sf::sleep(sf::seconds(2));
			continue;
		}
        //some other error
		else if (res.code != 200) {
			std::cout << "ERROR (song change): " << res.code << "\n";
            sf::sleep(sf::seconds(2));
            continue;
		}

		auto json = res.toJson();

        if (json["currently_playing_type"] != "track") {
            currentSong_ = json["currently_playing_type"];
            currentLyrics_ = { {"No Lyrics For This Type Of Content", 0} };
            progress_ = 0, duration_ = 0;
            currentLine_ = 0;

            sf::sleep(sf::seconds(2));
            continue;
        }

        //new song detected
        if (json["item"]["name"] != currentSong_) {
		    currentSong_ = json["item"]["name"];
            currentLine_ = 0;
            duration_ = json["item"]["duration_ms"];

            //request lyrics from LRCLIB
			Request lReq = Request(Request::Methods::GET);
            lReq.url = "https://lrclib.net/api/get?track_name=" + format(currentSong_) + 
                "&artist_name=" + format(json["item"]["artists"][0]["name"]) + 
                "&duration=" + std::to_string(int(json["item"]["duration_ms"] / 1000));
            std::cout << lReq.url << "\n";
			res = CurlWrapper::send(lReq);

			//no lyrics found
            if (res.code != 200 || res.toJson()["syncedLyrics"].is_null())
                currentLyrics_ = { { "No Lyrics", 0 } };
			//process raw lyrics
            else {
                currentLyrics_ = { { "", 0 } };
                std::istringstream stream(std::string(res.toJson()["syncedLyrics"]));
                std::string line;
                while (std::getline(stream, line)) {
                    if (line.empty()) 
                        continue;

					size_t time = size_t(60'000 * std::stoi(line.substr(1, line.find_first_of(':'))));
					line = line.substr(line.find_first_of(':') + 1, line.size());
					time += size_t(1'000 * std::stoi(line.substr(0, line.find_first_of('.'))));
					line = line.substr(line.find_first_of('.') + 1, line.size());
					time += size_t(10 * std::stoi(line.substr(0, line.find_first_of(']'))));
					line = line.substr(line.find_first_of(']') + 1, line.size());

                    if (line[0] == ' ')
                        line = line.substr(1, line.size());

					currentLyrics_.push_back({ line, std::max(0, int(time) - 650) });
                }
            }

            drawOverlay();
        }

		progress_ = json["progress_ms"];
		isPlaying_ = json["is_playing"];

        //sleep before checking again
        sf::sleep(sf::seconds(2));
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

		size_t i = 0;
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
