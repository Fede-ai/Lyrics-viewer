#include "overlay.hpp"
#include "curlwrapper.hpp"
#include "cefapp.hpp"

#pragma comment (lib, "dwmapi.lib")

void getAuthInfo(std::string& info) 
{
    //create a named pipe
    HANDLE hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\MyNamedPipe"), PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "error 102: " << GetLastError() << "\n";
        return;
    }

    //wait for a client to connect
    if (ConnectNamedPipe(hPipe, NULL)) {
        char buffer[1024];
        DWORD dwRead;
        BOOL success = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL);

        if (success) {
            buffer[dwRead] = '\0'; //null terminate the string
            info = buffer;
        }
        else
            std::cerr << "error 103: " << GetLastError() << "\n";
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

int main() 
{
    char buf[_MAX_PATH + 1];
    GetModuleFileNameA(NULL, buf, _MAX_PATH);
    std::string dir = std::string(buf).substr(0, std::string(buf).find_last_of('\\'));

    CefMainArgs mainArgs;
    CefSettings settings;

    settings.chrome_runtime = true;
    settings.no_sandbox = true;
    settings.log_severity = LOGSEVERITY_FATAL;
    CefString(&settings.root_cache_path).FromASCII((dir + std::string("/cache")).c_str());
    CefString(&settings.log_file).FromASCII((dir + std::string("/log.log")).c_str());

    // SimpleApp implements application-level callbacks for the browser process. It will
    // create the first browser instance in OnContextInitialized() after CEF has initialized.
    CefRefPtr<SimpleApp> app = new SimpleApp();

    // CEF applications have multiple sub-processes (render, GPU, etc) that share the same executable. 
    // This function checks the command-line and, if this is a sub-process, executes the appropriate logic.
    int exit_code = CefExecuteProcess(mainArgs, app.get(), nullptr);
    //The sub-process has completed so return here.
    if (exit_code >= 0)
        return exit_code;

    // Initialize the CEF browser process. May return false if initialization fails
    // or if early exit is desired (for example, due to process singleton relaunch behavior).
    if (!CefInitialize(mainArgs, settings, app.get(), nullptr))
        return CefGetExitCode();

    std::string info;
    std::thread getAuthInfoThread(getAuthInfo, std::ref(info));

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called
    CefRunMessageLoop();

    CefShutdown();

    getAuthInfoThread.join();
    auto code = info.substr(0, info.find_first_of('+'));
    auto codeVerifier = info.substr(code.length() + 1);

    CurlWrapper curl;
    Request req(Request::Methods::POST);
    req.headers = { "Content-Type: application/x-www-form-urlencoded" };
    req.url = "https://accounts.spotify.com/api/token";
    req.body = "grant_type=authorization_code&code=" + code + "&redirect_uri=http://fede-ai.github.io/Lyrics-viewer/redirect.html" +
        "&client_id=244ba241897d4c969d1260ad0c844f91&code_verifier=" + codeVerifier;

    auto res = curl.send(req).toJson();
    std::string token = res["access_token"];
    std::erase(token, '"');
    std::string refresh = res["refresh_token"];
    std::erase(refresh, '"');

    Request r = Request(Request::Methods::GET);
    r.url = "https://api.spotify.com/v1/me";
    r.headers = { "Authorization: Bearer " + token };
    std::cout << curl.send(r).body;

    //Overlay ol;
    //int state = ol.run();
    //return state;

    return 0;
}