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
            //connect to the named pipe
            HANDLE hPipe = CreateFile(TEXT("\\\\.\\pipe\\MyNamedPipe"), 
                GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

            if (hPipe == INVALID_HANDLE_VALUE) {
                std::cout << "error 100: " << GetLastError() << "\n";
                return true;
            }

            std::string m = arguments[1]->GetStringValue();
            const char* msg = m.c_str();
            DWORD dwWritten;
            BOOL success = WriteFile(hPipe, msg, (DWORD)strlen(msg), &dwWritten, NULL);

            if (!success) {
                std::cout << "error 101: " << GetLastError() << "\n";
                CloseHandle(hPipe);
                return true;
            }

            CloseHandle(hPipe);

            return true;
        }
        return false;
    }

private:
    IMPLEMENT_REFCOUNTING(MyV8Handler);
};