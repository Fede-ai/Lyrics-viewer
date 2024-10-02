#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "include/cef_client.h"

class SimpleClient : public CefClient,
    public CefRenderHandler,
    public CefLifeSpanHandler {
public:
    SimpleClient() {};
    ~SimpleClient() override {};

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
    bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX,
        int viewY, int& screenX, int& screenY) override;

private:
    void handleWindow();

    sf::Shader shaderBR;
    sf::Vector2i size_ = sf::Vector2i(0, 0);
    sf::RenderWindow window_;
    CefRefPtr<CefBrowser> browser_;

    IMPLEMENT_REFCOUNTING(SimpleClient);
};