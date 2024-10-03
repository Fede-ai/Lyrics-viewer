#include "authclient.hpp"

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"

std::vector<CefRefPtr<CefBrowser>> AuthClient::browsers_ {};

void AuthClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CEF_REQUIRE_UI_THREAD();

    //std::cout << "OnTitleChange auth\n";

    if (auto browser_view = CefBrowserView::GetForBrowser(browser)) {
        //set the title of the window using the Views framework.
        CefRefPtr<CefWindow> window = browser_view->GetWindow();
        if (window)
            window->SetTitle(title);
    }
}

void AuthClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    browsers_.push_back(browser);
    
    std::cout << "OnAfterCreated auth, size = " << browsers_.size() << "\n";
}

void AuthClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    for (int i = 0; i < browsers_.size(); i++) {
        if (browsers_[i]->IsSame(browser)) {
            browsers_.erase(browsers_.begin() + i);
            break;
        }
    }

    std::cout << "OnBeforeClose auth, size = " << browsers_.size() << "\n";
}
