#include "overlay.hpp"
#include "curlwrapper.hpp"
#include <thread>

int performAuth() {
#ifndef NDEBUG
	return -1;
#else
	//run the auth program as a separate process (therefore return immediately)
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	std::string authProg = "Lyrics-viewer-Auth/Lyrics-viewer-Auth.exe";
	if (!CreateProcessA(NULL, authProg.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		return -2;
	else {
		//close process and thread handles
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return 0;
	}
#endif
}

int main(int argc, char* argv[])
{
	if (argc != 4)
		return performAuth();
	
	std::string at = "", rt = "", path;
	at = argv[1];
	rt = argv[2];
	path = argv[3];

    Overlay overlay(at, rt, path);
    overlay.run();

    return 0;
}