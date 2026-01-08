/*
    Project: GDI Audio Visualizer (BPM Sync)
    Language: C++
    Description: A graphical experiment using Windows GDI and WASAPI for audio synchronization.
    Controls: Press [CTRL + ALT + B] to force exit.
*/

#include <windows.h>
#include <cmath>
#include <iostream>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

// Linker directives for Visual Studio
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")

// --- CONFIGURATION ---
const int BPM = 75; // Target beat per minute
const double BEAT_INTERVAL = 60000.0 / BPM; 
const double PI = 3.14159265359;

// Global variables for screen metrics
int scrw, scrh;
int centerX, centerY;
double intensity = 0.0;
int beatCounter = 0;

// Initialize screen dimensions
void Setup() {
    scrw = GetSystemMetrics(SM_CXSCREEN);
    scrh = GetSystemMetrics(SM_CYSCREEN);
    centerX = scrw / 2;
    centerY = scrh / 2;
}

// Hide the console window completely
void GoStealth() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}

// Check for the emergency exit key combination: CTRL + ALT + B
bool CheckExit() {
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
        (GetAsyncKeyState(VK_MENU) & 0x8000) && 
        (GetAsyncKeyState('B') & 0x8000)) 
    {
        return true;
    }
    return false;
}

// Set System Volume to 100% using WASAPI
void SetMaxVolume() {
    HRESULT hr;
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (SUCCEEDED(hr)) {
        IMMDevice *defaultDevice = NULL;
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &defaultDevice);
        if (SUCCEEDED(hr)) {
            IAudioEndpointVolume *endpointVolume = NULL;
            hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
            if (SUCCEEDED(hr)) {
                endpointVolume->SetMasterVolumeLevelScalar(1.0f, NULL);
                endpointVolume->Release();
            }
            defaultDevice->Release();
        }
        deviceEnumerator->Release();
    }
    CoUninitialize();
}

// --- INTRO SEQUENCE ---
void PlayIntroSequence(HDC hdc) {
    // 1. CIRCLE (Error Icons)
    int radius = 300;
    for (double angle = 0; angle < 2 * PI; angle += 0.1) {
        if (CheckExit()) exit(0);
        int x = centerX + (int)(radius * cos(angle));
        int y = centerY + (int)(radius * sin(angle));
        DrawIcon(hdc, x - 16, y - 16, LoadIcon(NULL, IDI_ERROR));
        Sleep(10); 
    }

    // 2. SQUARE (Warning Icons)
    int size = 350; 
    // Top & Bottom
    for (int x = centerX - size; x <= centerX + size; x += 40) {
        if (CheckExit()) exit(0);
        DrawIcon(hdc, x, centerY - size, LoadIcon(NULL, IDI_WARNING));
        DrawIcon(hdc, x, centerY + size, LoadIcon(NULL, IDI_WARNING));
        Sleep(10);
    }
    // Left & Right
    for (int y = centerY - size; y <= centerY + size; y += 40) {
        if (CheckExit()) exit(0);
        DrawIcon(hdc, centerX - size, y, LoadIcon(NULL, IDI_WARNING)); 
        DrawIcon(hdc, centerX + size, y, LoadIcon(NULL, IDI_WARNING)); 
        Sleep(10);
    }

    // 3. CENTER (Info Icons)
    for (int x = centerX - 60; x <= centerX + 60; x+=60) {
        for (int y = centerY - 60; y <= centerY + 60; y+=60) {
             DrawIcon(hdc, x, y, LoadIcon(NULL, IDI_INFORMATION));
             Sleep(50);
        }
    }
}

// Draw a trail following the mouse cursor (No Text)
void DrawCursorTrail(HDC hdc) {
    POINT cursor;
    GetCursorPos(&cursor); 
    // Just the icon, purely visual
    DrawIcon(hdc, cursor.x, cursor.y, LoadIcon(NULL, IDI_ERROR));
}

// Main rendering function
void RenderFrame(HDC hdc) {
    // Calculate zoom based on beat intensity
    int zoomAmount = (int)(intensity * 35);

    if (intensity > 0.05) {
        StretchBlt(
            hdc, 
            zoomAmount, zoomAmount,              
            scrw - (2 * zoomAmount),             
            scrh - (2 * zoomAmount),             
            hdc, 
            0, 0,                                
            scrw, scrh, 
            SRCCOPY
        );
    }
    
    // Draw the cursor trail on top
    DrawCursorTrail(hdc);
}

int main() {
    // --- PHASE 1: STEALTH & WAIT ---
    GoStealth();
    Setup();

    // Wait 5 seconds silently
    for(int i=0; i<50; i++) {
        if (CheckExit()) return 0;
        Sleep(100); 
    }

    HDC hdc = GetDC(0);

    // --- PHASE 2: VISUAL INTRO ---
    PlayIntroSequence(hdc);

    // --- PHASE 3: AUDIO & CHAOS ---
    SetMaxVolume(); 
    PlaySound(TEXT("pigstep.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    
    DWORD startTime = GetTickCount();
    DWORD lastBeatTime = 0;

    while (true) {
        // Emergency Exit Check
        if (CheckExit()) break;

        // Beat Calculation
        DWORD currentTime = GetTickCount();
        DWORD elapsedTime = currentTime - startTime;

        if (elapsedTime - lastBeatTime >= BEAT_INTERVAL) {
            lastBeatTime += (DWORD)BEAT_INTERVAL;
            beatCounter++;

            // Trigger beat every 2 counts (Half-time/Heavy feel)
            if (beatCounter % 2 == 0) {
                intensity = 1.0; 
                // Optional: Re-force volume
                // SetMaxVolume(); 
            }
        }

        RenderFrame(hdc);

        // Smooth Decay
        intensity *= 0.96; 
        if (intensity < 0.01) intensity = 0;

        // Frame limiter (~30 FPS)
        Sleep(30); 
    }

    // Cleanup
    ReleaseDC(0, hdc);
    PlaySound(NULL, 0, 0);
    return 0;
}
