#pragma once
#include "cefclient.hpp"
#include "include/cef_app.h"

class SimpleApp : public CefApp, 
    public CefBrowserProcessHandler,
    public CefRenderProcessHandler {
public:
    SimpleApp() = default;

    // CefApp methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

    // CefRenderProcessHandler methods:
    void OnContextCreated(CefRefPtr<CefBrowser> browser, 
        CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;

    // CefBrowserProcessHandler methods:
    void OnContextInitialized() override;

private:
    CefRefPtr<SimpleClient> client_;

    IMPLEMENT_REFCOUNTING(SimpleApp);
};
