#pragma once
#include <iostream>
#include "include/cef_client.h"

class AuthClient : public CefClient,
    public CefDisplayHandler,
    public CefLifeSpanHandler {
public:
    AuthClient() {};
    ~AuthClient() override {};

    // CefClient methods:
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    // CefDisplayHandler methods:
    void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;

    // CefLifeSpanHandler methods:
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    static std::vector<CefRefPtr<CefBrowser>> getBrowsers() {
        return browsers_;
    }

private:
    static std::vector<CefRefPtr<CefBrowser>> browsers_;

    IMPLEMENT_REFCOUNTING(AuthClient);
};