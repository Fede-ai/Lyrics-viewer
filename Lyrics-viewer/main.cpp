#include "overlay.h"
#include "curlwrapper.h"

int main()
{
    std::string clientId = "244ba241897d4c969d1260ad0c844f91";
    std::string clientSecret = "a3a3471cde7d440cb3f0a6c6f75f6a2e";
    
    std::string url = "http://localhost:8000/redirect.html";
    ShellExecuteA(0, 0, url.c_str(), 0, 0, SW_SHOW);

	Overlay ol;
	return ol.run();
}