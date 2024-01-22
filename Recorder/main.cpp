#include <Windows.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <iostream>
#include <fstream>

int main() 
{
    // Initialize COM library
    CoInitialize(nullptr);
    IMMDeviceEnumerator* pEnumerator = nullptr;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    IMMDevice* pDevice = nullptr;
    pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    IAudioClient* pAudioClient = nullptr;
    pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient);
    WAVEFORMATEX* pWaveFormat;
    pAudioClient->GetMixFormat(&pWaveFormat);
    pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, pWaveFormat, nullptr);
    // Get the buffer size
    UINT32 bufferSize;
    pAudioClient->GetBufferSize(&bufferSize);
    // Get the capture client
    IAudioCaptureClient* pCaptureClient = nullptr;
    pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    // Open a file to write captured audio
    std::ofstream outputFile("audio.wav", std::ios::binary);
    // Start capturing audio
    pAudioClient->Start();


    UINT32 numFramesAvailable;
    BYTE* pData;
    DWORD flags;

    while (true) {
        // Wait for the next available capture frame
        pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
        // Write captured audio to the file
        if (numFramesAvailable > 0) {
            outputFile.write(reinterpret_cast<const char*>(pData), numFramesAvailable * pWaveFormat->nBlockAlign);
            pCaptureClient->ReleaseBuffer(numFramesAvailable);
        }

        // Check for the end of audio stream
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            break;
        }
    }

    // Stop capturing audio
    pAudioClient->Stop();

    // Clean up
    outputFile.close();
    pCaptureClient->Release();
    CoTaskMemFree(pWaveFormat);
    pAudioClient->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();
}