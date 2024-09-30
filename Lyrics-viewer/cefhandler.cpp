#include <SFML/Graphics.hpp>
#include "cefhandler.hpp"

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    browser_ = browser;
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed description of this
    // process.

    // Set a flag to indicate that the window close should be allowed.
    is_closing_ = true;

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    CefQuitMessageLoop();
}

void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    rect = CefRect(0, 0, 500, 300);

    std::cout << "SIZE\n";
}

void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, 
    const RectList& dirtyRects, const void* buffer, int width, int height)
{

    sf::Image img;
    img.create(width, height);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            char r = ((char*)(buffer))[4 * (y * width + x) + 2];
            char g = ((char*)(buffer))[4 * (y * width + x) + 1];
            char b = ((char*)(buffer))[4 * (y * width + x)];
            char a = ((char*)(buffer))[4 * (y * width + x) + 3];

            img.setPixel(x, y, sf::Color(r, g, b, a));
        }
    }
    static int i = 0;
    img.saveToFile("C:\\Users\\feder\\Desktop\\images\\img" + std::to_string(i) + ".png");
    i++;

    std::cout << "PAINT\n";
}

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    if (hwnd)
        SetWindowText(hwnd, std::wstring(title).c_str());
}