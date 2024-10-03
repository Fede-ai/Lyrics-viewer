#pragma once
#include <iostream>
#include "include/cef_client.h"

class WinlessClient : public CefClient,
    public CefRenderHandler,
    public CefLifeSpanHandler {
public:
    WinlessClient() {};
    ~WinlessClient() override {};

    // CefClient methods:
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    // CefLifeSpanHandler methods:
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefRenderHandler methods:
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, 
        const RectList& dirtyRects, const void* buffer, int width, int height) override;

private:
    CefRefPtr<CefBrowser> browser_;

    IMPLEMENT_REFCOUNTING(WinlessClient);
};