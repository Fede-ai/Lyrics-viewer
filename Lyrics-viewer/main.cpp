#include "overlay.h"
#include "curlwrapper.h"

int main()
{    
    std::string url = "http://fede-ai.github.io/Lyrics-viewer/redirect.html";
    ShellExecuteA(0, 0, url.c_str(), 0, 0, SW_SHOW);

	Overlay ol;
	return ol.run();
}