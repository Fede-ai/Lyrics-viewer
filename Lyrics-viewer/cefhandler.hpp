#pragma once
#include <iostream>
#include "include/cef_client.h"

class SimpleHandler : public CefClient,
    public CefRenderHandler,
    public CefLifeSpanHandler,
    public CefLoadHandler {
public:
    explicit SimpleHandler() {};
    ~SimpleHandler() override {};

    // CefClient methods:
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

    // CefLifeSpanHandler methods:
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, 
        const RectList& dirtyRects, const void* buffer, int width, int height) override;

    void OnAcceleratedPaint(CefRefPtr< CefBrowser > browser, PaintElementType type,
        const RectList& dirtyRects, const CefAcceleratedPaintInfo& info) override {
        std::cout << "PAINTTTTTTTTTTTTTTTTT\n";
    }
    

    bool IsClosing() const { return is_closing_; }

private:
    // Platform-specific implementation.
    void PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title);

    CefRefPtr<CefBrowser> browser_;
    bool is_closing_ = false;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleHandler);
};