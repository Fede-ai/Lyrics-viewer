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

    settings.chrome_runtime = true;
    settings.no_sandbox = true;
    CefString(&settings.root_cache_path).FromASCII((dir + std::string("/cache")).c_str());
    CefString(&settings.log_file).FromASCII((dir + std::string("/log.log")).c_str());

    CefRefPtr<SimpleApp> app = new SimpleApp();
    if (!CefInitialize(mainArgs, settings, app, nullptr))
        std::exit(CefGetExitCode());

    // Run the CEF message loop. This will block until CefQuitMessageLoop() is called
    CefRunMessageLoop();

    CefShutdown();

    Overlay ol;
    return ol.run();
}