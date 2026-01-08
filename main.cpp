/*
    Project: CBA Visualizer (Fixed)
    Language: C++
    Author: CBA
    Description: GDI Visualizer with Signature.
    Exit: [CTRL + ALT + B]
*/

#include <windows.h>
#include <cmath>
#include <iostream>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

// Kütüphaneler
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")

// --- AYARLAR ---
const int BPM = 75; 
const double BEAT_INTERVAL = 60000.0 / BPM; 

// Global Değişkenler
int vX, vY, vW, vH; 
int centerX, centerY;
double intensity = 0.0;
int beatCounter = 0;

void Setup() {
    vX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    vY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    vW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    vH = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    centerX = vX + (vW / 2);
    centerY = vY + (vH / 2);
}

void GoStealth() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}

bool CheckExit() {
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
        (GetAsyncKeyState(VK_MENU) & 0x8000) && 
        (GetAsyncKeyState('B') & 0x8000)) 
    {
        return true;
    }
    return false;
}

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

void DisintegrateEffect(HDC hdc, int durationMs) {
    DWORD start = GetTickCount();
    while (GetTickCount() - start < durationMs) {
        if (CheckExit()) exit(0);
        int x = vX + (rand() % vW);
        int y = vY + (rand() % vH);
        int destX = vX + (rand() % vW);
        int destY = vY + (rand() % vH);
        BitBlt(hdc, destX, destY, 4, 4, hdc, x, y, SRCCOPY);
    }
}

// --- DÜZELTME: LPCSTR KULLANILDI (ANSI UYUMU) ---
void DrawPattern(HDC hdc, int startX, int startY, int pattern[5][5], LPCSTR iconID) {
    int spacing = 60; 

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            if (CheckExit()) exit(0);

            if (pattern[row][col] == 1) {
                int drawX = startX + (col * spacing);
                int drawY = startY + (row * spacing);
                
                // LoadIconA kullanarak tür uyuşmazlığını çözdük
                DrawIcon(hdc, drawX, drawY, LoadIconA(NULL, iconID));
                Sleep(30); 
            }
        }
    }
}

// --- CBA İMZASI (Matris Haritaları) ---
void DrawSignature(HDC hdc) {
    
    // C Harfi
    int patternC[5][5] = {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 1, 1}
    };

    // B Harfi
    int patternB[5][5] = {
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 0}
    };

    // A Harfi
    int patternA[5][5] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1}
    };

    int startY = centerY - 150; 
    
    // C - Error İkonu
    // Tür dönüşümü hatasını önlemek için (LPCSTR) cast ekliyoruz
    DrawPattern(hdc, centerX - 500, startY, patternC, (LPCSTR)IDI_ERROR);
    Sleep(200);

    // B - Warning İkonu
    DrawPattern(hdc, centerX - 150, startY, patternB, (LPCSTR)IDI_WARNING);
    Sleep(200);

    // A - Info İkonu
    DrawPattern(hdc, centerX + 200, startY, patternA, (LPCSTR)IDI_INFORMATION);
    Sleep(500);
}

void RenderRhythmFrame(HDC hdc) {
    int zoomAmount = (int)(intensity * 35);
    if (intensity > 0.05) {
        StretchBlt(hdc, vX + zoomAmount, vY + zoomAmount, vW - (2 * zoomAmount), vH - (2 * zoomAmount), hdc, vX, vY, vW, vH, SRCCOPY);
    }
    
    POINT cursor;
    GetCursorPos(&cursor); 
    DrawIcon(hdc, cursor.x, cursor.y, LoadIconA(NULL, (LPCSTR)IDI_ERROR));
}

int main() {
    GoStealth();
    Setup();
    HDC hdc = GetDC(NULL); 

    // 1. ADIM: ERİME EFEKTİ
    DisintegrateEffect(hdc, 5000);

    // 2. ADIM: CBA İMZASI
    DrawSignature(hdc);

    // 3. ADIM: MÜZİK VE DANS
    SetMaxVolume(); 
    PlaySound(TEXT("pigstep.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    
    DWORD startTime = GetTickCount();
    DWORD lastBeatTime = 0;

    while (true) {
        if (CheckExit()) break;

        DWORD currentTime = GetTickCount();
        DWORD elapsedTime = currentTime - startTime;

        if (elapsedTime - lastBeatTime >= BEAT_INTERVAL) {
            lastBeatTime += (DWORD)BEAT_INTERVAL;
            beatCounter++;
            if (beatCounter % 2 == 0) intensity = 1.0; 
        }

        RenderRhythmFrame(hdc);

        intensity *= 0.96; 
        if (intensity < 0.01) intensity = 0;
        Sleep(30); 
    }

    ReleaseDC(NULL, hdc);
    PlaySound(NULL, 0, 0);
    return 0;
}
