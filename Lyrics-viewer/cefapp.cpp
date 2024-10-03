#include "cefapp.hpp"
#include "binding.hpp"

#include "include/wrapper/cef_helpers.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"

namespace {
    //when using the Views framework this object provides the delegate
    //implementation for the CefWindow that hosts the Views-based browser
    class SimpleWindowDelegate : public CefWindowDelegate {
    public:
        SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
            :
            browserView(browser_view)
        {}

        void OnWindowCreated(CefRefPtr<CefWindow> window) override {
            static int numWindow = 0;

            RECT screen;
            GetWindowRect(GetDesktopWindow(), &screen);
            int xPos = int(screen.right / 2.f - screen.right / 5.6f - 10 * numWindow);
            int yPos = int(screen.bottom / 2.2f - screen.right / 5.f - 10 * numWindow);
            CefPoint position(xPos, yPos);
            window->SetPosition(position);

            //add the browser view and show the window.
            window->AddChildView(browserView);
            numWindow++;

            if (showState != CEF_SHOW_STATE_HIDDEN)
                window->Show();

            if (showState != CEF_SHOW_STATE_MINIMIZED && showState != CEF_SHOW_STATE_HIDDEN)
                browserView->RequestFocus();
        }

        void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
            browserView = nullptr;
        }

        bool CanClose(CefRefPtr<CefWindow> window) override {
            // Allow the window to close if the browser says it's OK.
            CefRefPtr<CefBrowser> browser = browserView->GetBrowser();
            if (browser)
                return browser->GetHost()->TryCloseBrowser();

            return true;
        }

        CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
            RECT screen;
            GetWindowRect(GetDesktopWindow(), &screen);
            return CefSize(int(screen.right / 2.8f), int(screen.right / 2.5f));
        }

        cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
            return showState;
        }

        cef_runtime_style_t GetWindowRuntimeStyle() override {
            return runtimeStyle;
        }

    private:
        CefRefPtr<CefBrowserView> browserView;
        const cef_runtime_style_t runtimeStyle = CEF_RUNTIME_STYLE_CHROME;
        const cef_show_state_t showState = CEF_SHOW_STATE_NORMAL;

        IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
        DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
    };

    class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
    public:
        SimpleBrowserViewDelegate() {};

        bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
            CefRefPtr<CefBrowserView> popup_browser_view,
            bool is_devtools) override {
            //create a new top-level Window for the popup. It will show itself after creation.
            CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(popup_browser_view));

            //we created the Window.
            return true;
        }

        cef_runtime_style_t GetBrowserRuntimeStyle() override {
            return runtimeStyle;
        }

    private:
        const cef_runtime_style_t runtimeStyle = CEF_RUNTIME_STYLE_CHROME;

        IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
        DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
    };
}

void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<SimpleV8Handler> handler = new SimpleV8Handler();

    CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("sendToCpp", handler);
    global->SetValue("sendToCpp", func, V8_PROPERTY_ATTRIBUTE_NONE);
}

void SimpleApp::OnContextInitialized()
{
    //CEF_REQUIRE_UI_THREAD();
    //
    //std::string url = "http://fede-ai.github.io/Lyrics-viewer/redirect.html";
    //CefBrowserSettings browserSettings;
    //
    //CefWindowInfo windowInfo;
    //windowInfo.SetAsWindowless(nullptr);
    //
    //client_ = new WinlessClient();
    //CefBrowserHost::CreateBrowserSync(windowInfo, client_, url, browserSettings, nullptr, nullptr);


    CEF_REQUIRE_UI_THREAD();

    std::string url = "http://fede-ai.github.io/Lyrics-viewer/redirect.html";
    CefBrowserSettings browserSettings;

    authClient_ = new AuthClient();

    //create the BrowserView.
    CefRefPtr<CefBrowserView> browserView = CefBrowserView::CreateBrowserView(
        authClient_, url, browserSettings, nullptr, nullptr, new SimpleBrowserViewDelegate());

    //create the Window. it will show itself after creation.
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browserView));
}

void SimpleApp::closeAuthWindows()
{
    for (int i = 0; i < AuthClient::GetBrowsers().size(); i++) {
        AuthClient::GetBrowsers()[i]->GetHost()->CloseBrowser(true);
    }
}
