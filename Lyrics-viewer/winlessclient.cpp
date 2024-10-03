#include "winlessclient.hpp"

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

void WinlessClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    browser_ = browser;

    std::cout << "OnAfterCreated winless\n";
}

bool WinlessClient::DoClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed description of this
    // process.

    std::cout << "DoClose winless\n";

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return true;
}

void WinlessClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    std::cout << "OnBeforeClose winless\n";
}

void WinlessClient::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    CEF_REQUIRE_UI_THREAD();

    rect.Set(0, 0, 0, 0);
}

void WinlessClient::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, 
    const RectList& dirtyRects, const void* buffer, int width, int height)
{
    CEF_REQUIRE_UI_THREAD();
}
