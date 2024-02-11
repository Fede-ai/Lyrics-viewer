#include <Windows.h>
#include <iostream>
#include "recorder.h"
#include <thread>
#include <chrono>

void stop(Recorder& rec)
{
    std::this_thread::sleep_for(std::chrono::seconds(10));
    rec.stopRecording();
}

int main()
{
    Recorder recorder;

    for (int i = 0; i < 1; i++)
    {
        std::thread thr(&stop, std::ref(recorder));
        recorder.startRecording(const_cast<LPWSTR>(L"audio/audio.wav"));
        thr.join();
    }

    return 0;
}
