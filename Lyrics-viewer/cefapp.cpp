#include "cefapp.hpp"
#include "binding.hpp"

#include <string>
#include "include/wrapper/cef_helpers.h"

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

    std::string url = "http://fede-ai.github.io/Lyrics-viewer/redirect.html";

    // Specify CEF browser settings here.
    CefBrowserSettings browserSettings;

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(nullptr);

    CefRefPtr<SimpleHandler> handler = new SimpleHandler();
    CefBrowserHost::CreateBrowserSync(windowInfo, handler, url, browserSettings, nullptr, nullptr);
}