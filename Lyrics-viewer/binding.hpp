#pragma once
#include "include/cef_v8.h"
#include <iostream>

class SimpleV8Handler : public CefV8Handler {
public:
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override {
        if (name == "sendToCpp" && arguments.size() == 1 && arguments[0]->IsString())
        {
            //connect to the named pipe
            HANDLE hPipe = CreateFile(TEXT("\\\\.\\pipe\\firstToken"), 
                GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

            //failed to connect to the pipe (error 101)
            if (hPipe == INVALID_HANDLE_VALUE) {
                std::cerr << "error 101: " << GetLastError() << "\n";
                return false;
            }

            std::string m = arguments[0]->GetStringValue();
            const char* msg = m.c_str();
            DWORD bytesWritten;
            BOOL success = WriteFile(hPipe, msg, (DWORD)strlen(msg), &bytesWritten, NULL);

            //failed to send message through pipe (error 102)
            if (!success) {
                std::cerr << "error 102: " << GetLastError() << "\n";
                CloseHandle(hPipe);
                return false;
            }

            CloseHandle(hPipe);

            return true;
        }

        if (name == "getToken" && arguments.size() == 0)
        {
            //connect to the named pipe
            HANDLE hPipe = CreateFile(TEXT("\\\\.\\pipe\\shareToken"),
                GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

            //failed to connect to the pipe (error 105)
            if (hPipe == INVALID_HANDLE_VALUE) {
                std::cerr << "error 105: " << GetLastError() << "\n";
                return false;
            }

            char buffer[512] = {};
            DWORD bytesRead;
            //failed to read the message (error 107)
            if (!ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                std::cerr << "error 107: " << GetLastError() << "\n";
                CloseHandle(hPipe);
                return false;
            }

            buffer[bytesRead] = '\0'; //null terminate the string
            std::string str = buffer;
            retval = CefV8Value::CreateString(buffer);

            return true;
        }

        if (name == "printCpp" && arguments.size() == 1 && arguments[0]->IsString())
        {
            std::cout << "javascript says: " << arguments[0]->GetStringValue() << "\n";
            return true;
        }

        std::cerr << "error 150: function name = " << name << "\n";

        return false;
    }

private:
    IMPLEMENT_REFCOUNTING(SimpleV8Handler);
};