// AudioEndpointConsoleApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "CAudioEndpointVolumeCallback.h"
#include <Audioclient.h>

GUID g_guidMyContext = GUID_NULL;

static IAudioEndpointVolume* g_pEndptVol = NULL;
static ISimpleAudioVolume* g_pSimpleVol = NULL;
static IChannelAudioVolume* g_pChannelVol = NULL;
static IAudioStreamVolume* g_pAudioSteamVol = NULL;

int main()
{
    HRESULT hr = S_OK;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    CAudioEndpointVolumeCallback EPVolEvents;
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf_s("CoInitialize failed\n");
        return 0;
    }

    hr = CoCreateGuid(&g_guidMyContext);
    if (FAILED(hr)) {
        printf_s("Create context failed\n");
        return 0;
    }

    // Get enumerator for audio endpoint devices.
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
        NULL, CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator);
    if (FAILED(hr)) {
        printf_s("Create instance failed\n");
        return 0;
    }

    // Get default audio device.
    //TODO: check input devices via eCapture
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    if (FAILED(hr)) {
        printf_s("Get audio endpoint failed\n");
        return 0;
    }

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume),
        CLSCTX_ALL, NULL, (void**)&g_pEndptVol);
    if (FAILED(hr)) {
        printf_s("Get audio endpoint failed\n");
        return 0;
    }

    // Get audio client interface
    hr = pDevice->Activate(__uuidof(IAudioClient),
        CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) {
        printf_s("Get audio client interface failed\n");
        return 0;
    }

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        printf_s("Get mix format failed\n");
        return 0;
    }

#define REFTIMES_PER_SEC  10000000
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0,
        hnsRequestedDuration, 0,
        pwfx, NULL);
    if (FAILED(hr)) {
        printf_s("Audio client init failed\n");
        return 0;
    }

    hr = pAudioClient->GetService(__uuidof(ISimpleAudioVolume), (void**)&g_pSimpleVol);
    if (FAILED(hr)) {
        g_pSimpleVol = NULL;
    }
    else {
        printf_s("Get simple audio vol: %p\n", g_pSimpleVol);
    }

    hr = pAudioClient->GetService(__uuidof(IChannelAudioVolume), (void**)&g_pChannelVol);
    if (FAILED(hr)) {
        g_pChannelVol = NULL;
    }
    else {
        printf_s("Get channel audio vol: %p\n", g_pChannelVol);
    }

    hr = pAudioClient->GetService(__uuidof(IAudioStreamVolume), (void**)&g_pAudioSteamVol);
    if (FAILED(hr)) {
        g_pAudioSteamVol = NULL;
    }
    else {
        printf_s("Get audio stream vol: %p\n", g_pAudioSteamVol);
    }


    hr = g_pEndptVol->RegisterControlChangeNotify(
        (IAudioEndpointVolumeCallback*)&EPVolEvents);
    if (FAILED(hr)) {
        printf_s("Register change notify failed\n");
    }

    UINT channels = 0;
    hr = g_pEndptVol->GetChannelCount(&channels);
    if (FAILED(hr)) {
        printf_s("Get channel count failed\n");
    }
    else {
        printf_s("Get channel count: %d\n", channels);
    }

    printf_s("Simple volume interface, set 50%% volume and muted\n");
    g_pSimpleVol->SetMasterVolume(0.5f, NULL);
    g_pSimpleVol->SetMute(TRUE, NULL);

    system("pause");

    printf_s("Simple volume interface, set 100%% volume and unmuted\n");
    g_pSimpleVol->SetMasterVolume(1.0f, NULL);
    g_pSimpleVol->SetMute(FALSE, NULL);

    system("pause");

    channels = 0;
    g_pChannelVol->GetChannelCount(&channels);
    printf_s("Channel volume interface, channel count: %d\n", channels);

    float vol = 0.5f;
    g_pChannelVol->SetAllVolumes(channels, &vol, NULL);
    printf_s("Channel volume interface, set 50%% volume\n");

    system("pause");

    vol = 1.0f;
    g_pChannelVol->SetAllVolumes(channels, &vol, NULL);
    printf_s("Channel volume interface, set 100%% volume\n");

    system("pause");

    g_pAudioSteamVol->GetChannelCount(&channels);
    printf_s("Audio stream volume interface, channel count: %d\n", channels);

    vol = 0.5f;
    g_pAudioSteamVol->SetAllVolumes(channels, &vol);
    printf_s("Audio stream volume interface, set 50%% volume\n");

    system("pause");

    vol = 1.0f;
    g_pAudioSteamVol->SetAllVolumes(channels, &vol);
    printf_s("Audio stream volume interface, set 100%% volume\n");

    system("pause");

    printf_s("Change windows setting, debug console will print notify msg\n");
    system("pause");

    if (g_pEndptVol != NULL)
    {
        g_pEndptVol->UnregisterControlChangeNotify(
            (IAudioEndpointVolumeCallback*)&EPVolEvents);
    }

    return 0;
}

