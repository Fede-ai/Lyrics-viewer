#include <Windows.h>
#include <iostream>
#include "recorder.h"
#include <thread>
#include <chrono>
#include <string>

void stop(Recorder& rec)
{
    std::this_thread::sleep_for(std::chrono::seconds(10));
    rec.stopRecording();
}

int main()
{
    //initialize COM on current thread
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
        std::cout << "COM initialization failed with error " << hr << "\n";

    //create recorder
    Recorder recorder;

    int i = 0;
    while (i++ < 2)
    {
        //create the file
        MMIOINFO mi = { 0 };
        WCHAR path[18];
        swprintf(path, sizeof(path) / sizeof(path[0]), L"audio/audio%d.wav", i);
        HMMIO file = mmioOpenW(path, &mi, MMIO_WRITE | MMIO_CREATE);
        
        std::thread stopThread(stop, std::ref(recorder));

        hr = recorder.startRecording(file);
        if (FAILED(hr))
            std::cout << "recording failed with error " << hr << "\n";
        stopThread.join();
    
        mmioClose(file, 0);
    }

    CoUninitialize();
    return 0;
}
