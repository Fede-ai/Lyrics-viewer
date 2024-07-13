#include "cefapp.hpp"
#include "binding.hpp"

#include <string>
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

namespace {
    // When using the Views framework this object provides the delegate
    // implementation for the CefWindow that hosts the Views-based browser.
    class SimpleWindowDelegate : public CefWindowDelegate {
    public:
        SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view, 
            cef_runtime_style_t runtime_style, 
            cef_show_state_t initial_show_state)
            : 
            browserView(browser_view),
            runtimeStyle(runtime_style),
            showState(initial_show_state) 
        {}

        void OnWindowCreated(CefRefPtr<CefWindow> window) override {
            RECT screen;
            GetWindowRect(GetDesktopWindow(), &screen);
            CefPoint position(int(screen.right / 2.f) - 300, int(screen.bottom / 2.2f) - 300);
            window->SetPosition(position);

            // Add the browser view and show the window.
            window->AddChildView(browserView);

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
            return CefSize(600, 600);
        }

        cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
            return showState;
        }

        cef_runtime_style_t GetWindowRuntimeStyle() override {
            return runtimeStyle;
        }

    private:
        CefRefPtr<CefBrowserView> browserView;
        const cef_runtime_style_t runtimeStyle;
        const cef_show_state_t showState;

        IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
        DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
    };

    class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
    public:
        explicit SimpleBrowserViewDelegate(cef_runtime_style_t runtime_style)
            : 
            runtime_style_(runtime_style) 
        {}

        bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
            CefRefPtr<CefBrowserView> popup_browser_view,
            bool is_devtools) override {
            // Create a new top-level Window for the popup. It will show itself after
            // creation.
            CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(
                popup_browser_view, runtime_style_, CEF_SHOW_STATE_NORMAL));

            // We created the Window.
            return true;
        }

        cef_runtime_style_t GetBrowserRuntimeStyle() override {
            return runtime_style_;
        }

    private:
        const cef_runtime_style_t runtime_style_;

        IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
        DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
    };
}

void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<MyV8Handler> handler = new MyV8Handler();

    CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("sendToCpp", handler);
    global->SetValue("sendToCpp", func, V8_PROPERTY_ATTRIBUTE_NONE);
}

void SimpleApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();

    cef_runtime_style_t runtimeStyle = CEF_RUNTIME_STYLE_CHROME;
    cef_show_state_t showState = CEF_SHOW_STATE_NORMAL;

    // SimpleHandler implements browser-level callbacks.
    CefRefPtr<SimpleHandler> handler(new SimpleHandler());

    // Specify CEF browser settings here.
    CefBrowserSettings browserSettings;

    std::string url = "http://fede-ai.github.io/Lyrics-viewer/redirect.html";

    // Create the BrowserView.
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        SimpleHandler::GetInstance(), url, browserSettings, nullptr, nullptr,
        new SimpleBrowserViewDelegate(runtimeStyle));
    
    // Create the Window. It will show itself after creation.
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view, runtimeStyle, showState));
}