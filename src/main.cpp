#include "overlay.hpp"
#include "curlwrapper.hpp"
#include <thread>

int main(int argc, char* argv[])
{
    if (argc != 3)
		return -1;
	
	std::string at = "", rt = "";
	at = argv[1];
	rt = argv[2];

    Overlay overlay(at, rt);
    overlay.run();

    return 0;
}