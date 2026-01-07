#include <windows.h>
#include <cmath>
#include <iostream>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

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

// SESİ %100 YAPAN KOD (Aynı kalıyor)
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

// --- MEMZ STİLİ ICON VE YAZI ÇİZME FONKSİYONU ---
void DrawMemzCursor(HDC hdc) {
    POINT cursor;
    GetCursorPos(&cursor); // Farenin nerede olduğunu bul

    // 1. İKON ÇİZİMİ
    // IDI_HAND (Hata işareti/X) veya IDI_WARNING (Ünlem) kullanabilirsin.
    // MEMZ genelde IDI_ERROR (X) kullanır.
    DrawIcon(hdc, cursor.x, cursor.y, LoadIcon(NULL, IDI_ERROR));

    // 2. YAZI ÇİZİMİ
    // Yazı arka planını şeffaf yap ki ikonun üstünü kapatmasın
    SetBkMode(hdc, TRANSPARENT); 
    
    // Kırmızı, kalın yazı
    SetTextColor(hdc, RGB(255, 0, 0)); 
    
    // Yazıyı farenin biraz sağına yaz
    TextOut(hdc, cursor.x + 32, cursor.y, TEXT("VIRUS DETECTED"), 14);
}

void RenderFrame(HDC hdc) {
    // Zoom/Dans Efekti
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

    // MEMZ Çizimini en son yapıyoruz ki diğer efektlerin ÜSTÜNDE görünsün
    DrawMemzCursor(hdc);
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
    Setup();
    
    // DİKKAT: Sesi fulle
    SetMaxVolume();

    PlaySound(TEXT("pigstep.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    
    DWORD startTime = GetTickCount();
    DWORD lastBeatTime = 0;
    HDC hdc = GetDC(0);

    while (true) {
        // ÇIKIŞ: CTRL + ALT + B
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
            (GetAsyncKeyState(VK_MENU) & 0x8000) && 
            (GetAsyncKeyState('B') & 0x8000)) 
        {
            break; 
        }

        // RİTİM HESAPLAMA
        DWORD currentTime = GetTickCount();
        DWORD elapsedTime = currentTime - startTime;

        if (elapsedTime - lastBeatTime >= BEAT_INTERVAL) {
            lastBeatTime += (DWORD)BEAT_INTERVAL;
            beatCounter++;

            if (beatCounter % 2 == 0) {
                intensity = 1.0; 
                // İstersen sesi sürekli zorla full yap:
                // SetMaxVolume(); 
            }
        }

        RenderFrame(hdc);

        // SMOOTH SÖNÜMLEME
        intensity *= 0.96; 
        if (intensity < 0.01) intensity = 0;

        Sleep(30); 
    }

    ReleaseDC(0, hdc);
    PlaySound(NULL, 0, 0);
    return 0;
}
