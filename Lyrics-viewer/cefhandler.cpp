#include "cefhandler.h"

#include <sstream>
#include <string>

#include "include/base/cef_callback.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace {
    SimpleHandler* g_instance = nullptr;

    // Returns a data: URI with the specified contents.
    std::string GetDataURI(const std::string& data, const std::string& mime_type) {
        return "data:" + mime_type + ";base64," + CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
    }
}

SimpleHandler::SimpleHandler()
{
    DCHECK(!g_instance);
    g_instance = this;
}

SimpleHandler::~SimpleHandler() 
{
    g_instance = nullptr;
}

// static
SimpleHandler* SimpleHandler::GetInstance() 
{
    return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) 
{
    CEF_REQUIRE_UI_THREAD();

    if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
        // Set the title of the window using the Views framework.
        CefRefPtr<CefWindow> window = browser_view->GetWindow();
        if (window)
            window->SetTitle(title);
    }
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    // Add to the list of existing browsers.
    browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed description of this
    // process.

    // Set a flag to indicate that the window close should be allowed.
    if (browser_list_.size() == 1)
        is_closing_ = true;

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    // Remove from the list of existing browsers.
    BrowserList::iterator bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browser_list_.erase(bit);
            break;
        }
    }

    // All browser windows have closed. Quit the application message loop.
    if (browser_list_.empty())
        CefQuitMessageLoop();
}

void SimpleHandler::ShowMainWindow() 
{
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::ShowMainWindow, this));
        return;
    }

    if (browser_list_.empty())
        return;

    auto main_browser = browser_list_.front();

    if (auto browser_view = CefBrowserView::GetForBrowser(main_browser)) {
        // Show the window using the Views framework.
        if (auto window = browser_view->GetWindow())
            window->Show();
    }
}

void SimpleHandler::CloseAllBrowsers(bool force_close) 
{
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
            force_close));
        return;
    }

    if (browser_list_.empty()) {
        return;
    }

    BrowserList::const_iterator it = browser_list_.begin();
    for (; it != browser_list_.end(); ++it)
        (*it)->GetHost()->CloseBrowser(force_close);
}

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    if (hwnd)
        SetWindowText(hwnd, std::wstring(title).c_str());
}