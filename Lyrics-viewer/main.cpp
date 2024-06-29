#include "overlay.h"
#include "curlwrapper.h"
#include "cefapp.h"

#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"

#pragma comment (lib, "dwmapi.lib")

int main() {
    char buf[_MAX_PATH + 1];
    GetModuleFileNameA(NULL, buf, _MAX_PATH);
    std::string dir = std::string(buf).substr(0, std::string(buf).find_last_of('\\'));

    CefMainArgs mainArgs;
    CefSettings settings;
    CefBrowserSettings s;

    settings.chrome_runtime = true;
    settings.no_sandbox = true;
    CefString(&settings.root_cache_path).FromASCII((dir + std::string("/cache")).c_str());
    CefString(&settings.log_file).FromASCII((dir + std::string("/log.log")).c_str());

    // SimpleApp implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefRefPtr<SimpleApp> app(new SimpleApp);

    // Initialize the CEF browser process. May return false if initialization
    // fails or if early exit is desired (for example, due to process singleton
    // relaunch behavior).
    if (!CefInitialize(mainArgs, settings, app.get(), nullptr))
        return CefGetExitCode();

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called
    CefRunMessageLoop();

    CefShutdown();
    
    Overlay ol;
    return ol.run();
}