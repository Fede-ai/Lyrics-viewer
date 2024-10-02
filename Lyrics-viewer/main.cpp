#include "cefapp.hpp"
#include "curlwrapper.hpp"

#pragma comment (lib, "dwmapi.lib")

void getAuthInfo(std::string& info) 
{
    //create a named pipe
    HANDLE hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\MyNamedPipe"), PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, 1024, 1024, 0, NULL);

    //failed to create the pipe (error 100)
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "error 100: " << GetLastError() << "\n";
        return;
    }

    //wait for a client to connect
    if (ConnectNamedPipe(hPipe, NULL)) {
        char buffer[1024] = {};
        DWORD dwRead;
        if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL)) {
            buffer[dwRead] = '\0'; //null terminate the string
            info = buffer;
        }
        //failed to read the message (error 103)
        else {
            std::cerr << "error 103: " << GetLastError() << "\n";
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    std::cout << info << "\n";
}

int main()
{
    char buf[_MAX_PATH + 1];
    GetModuleFileNameA(NULL, buf, _MAX_PATH);
    std::string dir = std::string(buf).substr(0, std::string(buf).find_last_of('\\'));

    CefSettings settings;
    settings.log_severity = LOGSEVERITY_WARNING;
    settings.no_sandbox = true;
    settings.windowless_rendering_enabled = true;
    CefString(&settings.root_cache_path).FromASCII((dir + std::string("/cache")).c_str());
    CefString(&settings.log_file).FromASCII((dir + std::string("/log.log")).c_str());

    CefRefPtr<SimpleApp> app = new SimpleApp();

    //CEF applications have multiple sub-processes (render, GPU, etc) that share the same executable. 
    //this function checks the command-line and, if this is a sub-process, executes the appropriate logic.
    CefMainArgs mainArgs;
    int exit_code = CefExecuteProcess(mainArgs, app, nullptr);
    //the sub-process has completed so return here
    if (exit_code >= 0) 
        return exit_code;    

    //initialize the CEF browser process. may return false if initialization fails
    //or if early exit is desired (for example, due to process singleton relaunch behavior).
    if (!CefInitialize(mainArgs, settings, app.get(), nullptr))
        return CefGetExitCode();

    std::string info;
    std::thread getAuthInfoThread(getAuthInfo, std::ref(info));

    //run the CEF message loop until CefQuitMessageLoop() is called
    CefRunMessageLoop();

    CefShutdown();

    getAuthInfoThread.join();
    //failed to authenticate spotify (error 200)
    if (info[0] == 'f') {
        std::cerr << "error 200: " << info.substr(1, info.size()) << "\n";
        return 200;
    }

    size_t sep = info.find_first_of('+');
    std::string accessToken = info.substr(1, sep - 1);
    std::string refreshToken= info.substr(sep + 1, info.size());

    //Request r = Request(Request::Methods::GET);
    //r.url = "https://api.spotify.com/v1/me";
    //r.headers = { "Authorization: Bearer " + accessToken };
    //std::cout << CurlWrapper::send(r).body;

    //Request r = Request(Request::Methods::POST);
    //r.url = "https://accounts.spotify.com/api/token";
    //r.headers = { "Content-Type: application/x-www-form-urlencoded" };
    //r.body = "grant_type=refresh_token&refresh_token=" + refreshToken + "&client_id=244ba241897d4c969d1260ad0c844f91";
    //std::cout << CurlWrapper::send(r).body;

    return 0;
}