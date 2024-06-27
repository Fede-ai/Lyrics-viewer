#include "overlay.h"
#include "curlwrapper.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "cefapp.h"

#pragma comment (lib, "dwmapi.lib")

int main() {
    // Provide CEF with command-line arguments.
    CefMainArgs mainArgs;

    // Specify CEF global settings here.
    CefSettings settings;

    settings.chrome_runtime = true;
    settings.no_sandbox = true;

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

    return 0;
    
    //Overlay ol;
    //return ol.run();
}