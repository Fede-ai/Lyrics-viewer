#pragma once
#include "include/cef_v8.h"
#include <iostream>

class MyV8Handler : public CefV8Handler {
public:
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override {
        if (name != "sendToCpp")
            return false;

        if (arguments.size() == 2 && arguments[0]->IsBool() && arguments[1]->IsString()) {
            std::cout << "message content: " << arguments[1]->GetStringValue() << "\n";
            return true;
        }
        return false;
    }

private:
    IMPLEMENT_REFCOUNTING(MyV8Handler);
};