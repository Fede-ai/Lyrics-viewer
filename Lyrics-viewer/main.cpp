#include "overlay.hpp"
#include "curlwrapper.hpp"
#include <thread>

int main()
{
    char buf[_MAX_PATH + 1];
    GetModuleFileNameA(NULL, buf, _MAX_PATH);
    std::string dir = std::string(buf).substr(0, std::string(buf).find_last_of('\\'));

    CefSettings settings;
    settings.log_severity = LOGSEVERITY_DEBUG;
    settings.no_sandbox = true;
    settings.windowless_rendering_enabled = true;
    CefString(&settings.root_cache_path).FromASCII((dir + std::string("/cache")).c_str());
    //CefString(&settings.log_file).FromASCII((dir + std::string("/log.log")).c_str());

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
    if (!CefInitialize(mainArgs, settings, app, nullptr))
        return CefGetExitCode();

    Overlay overlay(app);
    std::thread runOverlayThread(&Overlay::run, &overlay);

    //run the CEF message loop until CefQuitMessageLoop() is called
    CefRunMessageLoop();

    CefShutdown();

    //shutdown overlay forcingly
    if (overlay.isWaitingAuth()) {
        std::cerr << "error 201: auth window closed before login\n";
        runOverlayThread.detach();
    }
    //wait for overlay to close gracefully
    else {
        std::cout << "waiting for overlay thread to join\n";
        runOverlayThread.join();
    }

    return 0;
}