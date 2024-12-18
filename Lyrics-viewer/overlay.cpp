#include "overlay.hpp"
#include "curlwrapper.hpp"
#include <thread>

Overlay::Overlay(CefRefPtr<SimpleApp> inApp)
	:
	app_(inApp)
{
}

void Overlay::run()
{
    //if auth failed, close window withou launching player
    if (!getFirstToken()) {
        app_->closeAuthWindows(false);
        return;
    }

    //continuosly run the thread to send the token to the player
    std::thread shareThread(&Overlay::sendTokenToPlayer, this);
    shareThread.detach();

    //close auth an launch player
    sf::sleep(sf::seconds(0.2f));
    app_->closeAuthWindows(true);

    Request r = Request(Request::Methods::GET);
    r.url = "https://api.spotify.com/v1/me/player/currently-playing";
    r.headers = { "Authorization: Bearer " + accessToken_ };
    std::cout << CurlWrapper::send(r).body << "\n";

    //close player and shutdown cef
    app_->closePlayerBrowser();
    
    //Request r = Request(Request::Methods::POST);
    //r.url = "https://accounts.spotify.com/api/token";
    //r.headers = { "Content-Type: application/x-www-form-urlencoded" };
    //r.body = "grant_type=refresh_token&refresh_token=" + refreshToken + "&client_id=244ba241897d4c969d1260ad0c844f91";
    //std::cout << CurlWrapper::send(r).body;
}

bool Overlay::isWaitingAuth() const
{
    return waitingAuth_;
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
