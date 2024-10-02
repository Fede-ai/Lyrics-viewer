#include "cefapp.hpp"
#include "binding.hpp"

#include <string>
#include "include/wrapper/cef_helpers.h"

void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<SimpleV8Handler> handler = new SimpleV8Handler();

    CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("sendToCpp", handler);
    global->SetValue("sendToCpp", func, V8_PROPERTY_ATTRIBUTE_NONE);
}

void SimpleApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();

    std::string url = "http://fede-ai.github.io/Lyrics-viewer/redirect.html";

    CefBrowserSettings browserSettings;

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(nullptr);
    //windowInfo.external_begin_frame_enabled = true;

    client_ = new SimpleClient();
    CefBrowserHost::CreateBrowserSync(windowInfo, client_, url, browserSettings, nullptr, nullptr);
}
