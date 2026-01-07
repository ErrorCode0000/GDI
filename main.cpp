#include <windows.h>
#include <cmath>
#include <iostream>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

// Kütüphane Bağlantıları
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")

// --- AYARLAR ---
const int BPM = 75;
const double BEAT_INTERVAL = 60000.0 / BPM; 

int scrw, scrh;
double intensity = 0.0;
int beatCounter = 0;

void Setup() {
    scrw = GetSystemMetrics(SM_CXSCREEN);
    scrh = GetSystemMetrics(SM_CYSCREEN);
}

// PENCEREYİ GÖRÜNMEZ YAPAN FONKSİYON
void GoStealth() {
    HWND hWnd = GetConsoleWindow();
    // SW_HIDE: Pencereyi tamamen gizler (Taskbar'da bile görünmez)
    ShowWindow(hWnd, SW_HIDE);
}

// SESİ %100 YAPAN KOD
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

// MEMZ STİLİ İKON VE YAZI
void DrawMemzCursor(HDC hdc) {
    POINT cursor;
    GetCursorPos(&cursor); 

    // İkon çiz
    DrawIcon(hdc, cursor.x, cursor.y, LoadIcon(NULL, IDI_ERROR));

    // Yazı ayarları
    SetBkMode(hdc, TRANSPARENT); 
    SetTextColor(hdc, RGB(255, 0, 0)); // Kırmızı
    
    // Yazı fontunu büyütmek ve kalınlaştırmak istersen (İsteğe bağlı)
    /*
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(hdc, hFont);
    */

    TextOut(hdc, cursor.x + 32, cursor.y, TEXT("PIGSTEP.EXE"), 11);
    
    // Bellek sızıntısı olmaması için fontu silmek gerekir (CreateFont kullandıysan)
    // DeleteObject(hFont);
}

void RenderFrame(HDC hdc) {
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
    DrawMemzCursor(hdc);
}

int main() {
    // 1. ADIM: HEMEN GİZLEN (Stealth Mode)
    GoStealth();

    Setup();
    
    // 2. ADIM: SESİ FULLE
    SetMaxVolume();

    // 3. ADIM: MÜZİK
    PlaySound(TEXT("pigstep.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    
    DWORD startTime = GetTickCount();
    DWORD lastBeatTime = 0;
    HDC hdc = GetDC(0);

    while (true) {
        // ÇIKIŞ: CTRL + ALT + B (Burası artık HAYATİ önem taşıyor)
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
            (GetAsyncKeyState(VK_MENU) & 0x8000) && 
            (GetAsyncKeyState('B') & 0x8000)) 
        {
            break; 
        }

        DWORD currentTime = GetTickCount();
        DWORD elapsedTime = currentTime - startTime;

        if (elapsedTime - lastBeatTime >= BEAT_INTERVAL) {
            lastBeatTime += (DWORD)BEAT_INTERVAL;
            beatCounter++;

            // Yarı hızda vuruş
            if (beatCounter % 2 == 0) {
                intensity = 1.0; 
            }
        }

        RenderFrame(hdc);

        intensity *= 0.96; 
        if (intensity < 0.01) intensity = 0;

        Sleep(30); 
    }

    ReleaseDC(0, hdc);
    PlaySound(NULL, 0, 0);
    return 0;
}
