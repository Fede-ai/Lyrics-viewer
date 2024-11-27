#pragma once
#include "winlessclient.hpp"
#include "authclient.hpp"
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

    //called when a new browser window is created via the Chrome runtime UI.
    CefRefPtr<CefClient> GetDefaultClient() override {
        std::cout << "NEW DEFAULT\n";
        return new AuthClient();
    }

    void closeAuthWindows(bool auth);
    void closePlayerWindow();

private:
    void launchPlayerBrowser();

    CefRefPtr<WinlessClient> windowlessClient_;
    CefRefPtr<AuthClient> authClient_;

    IMPLEMENT_REFCOUNTING(SimpleApp);
};
