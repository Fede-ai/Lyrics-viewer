#include "overlay.hpp"
#include "curlwrapper.hpp"
#include "cefapp.hpp"

#pragma comment (lib, "dwmapi.lib")

int main() {   
    char buf[_MAX_PATH + 1];
    GetModuleFileNameA(NULL, buf, _MAX_PATH);
    std::string dir = std::string(buf).substr(0, std::string(buf).find_last_of('\\'));

    CefMainArgs mainArgs;
    CefSettings settings;

    settings.chrome_runtime = true;
    settings.no_sandbox = true;
    CefString(&settings.root_cache_path).FromASCII((dir + std::string("/cache")).c_str());
    CefString(&settings.log_file).FromASCII((dir + std::string("/log.log")).c_str());

    CefRefPtr<SimpleApp> app = new SimpleApp();
    if (!CefInitialize(mainArgs, settings, app, nullptr))
        std::exit(CefGetExitCode());

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called
    CefRunMessageLoop();

    Overlay ol;
    int state = ol.run();

    CefShutdown();

    return state;
}