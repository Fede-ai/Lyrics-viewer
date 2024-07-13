#pragma once
#include "cefhandler.hpp"

#include "include/cef_app.h"

// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp, 
    public CefBrowserProcessHandler,
    public CefRenderProcessHandler {
public:
    SimpleApp() = default;

    // CefApp methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }

    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) override;

    // CefBrowserProcessHandler methods:
    void OnContextInitialized() override;

    // Called when a new browser window is created via the Chrome runtime UI.
    CefRefPtr<CefClient> GetDefaultClient() override {
        return SimpleHandler::GetInstance();
    }

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleApp);
};
