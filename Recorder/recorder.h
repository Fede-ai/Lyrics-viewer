#pragma once
#include <Windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <iostream>

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
                  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
                  if ((punk) != NULL)  \
                    { (punk)->Release(); (punk) = NULL; }

class Recorder
{
public:
    void startRecording(LPWSTR path);
    void stopRecording();

private:
    bool isRecording = false;
    int callCount = 0;
    HRESULT record(HMMIO hFile);
    HRESULT writeWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO* pckRIFF, MMCKINFO* pckData);
    HRESULT finishWaveFile(HMMIO hFile, MMCKINFO* pckRIFF, MMCKINFO* pckData);
    HRESULT copyData(BYTE* pData, UINT32 NumFrames, WAVEFORMATEX* pwfx, HMMIO hFile);
};