/*
    Project: GDI Chaos Visualizer
    Language: C++
    Description: Advanced GDI manipulation with WASAPI audio sync.
    Exit: [CTRL + ALT + B]
*/

#include <windows.h>
#include <cmath>
#include <iostream>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

// Gerekli Kütüphaneler
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")

// --- AYARLAR ---
const int BPM = 75; 
const double BEAT_INTERVAL = 60000.0 / BPM; 
const double PI = 3.14159265359;

int scrw, scrh;
int centerX, centerY;
double intensity = 0.0;
int beatCounter = 0;

void Setup() {
    scrw = GetSystemMetrics(SM_CXSCREEN);
    scrh = GetSystemMetrics(SM_CYSCREEN);
    centerX = scrw / 2;
    centerY = scrh / 2;
}

void GoStealth() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}

// ACİL ÇIKIŞ (CTRL + ALT + B)
bool CheckExit() {
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
        (GetAsyncKeyState(VK_MENU) & 0x8000) && 
        (GetAsyncKeyState('B') & 0x8000)) 
    {
        return true;
    }
    return false;
}

// SES KONTROLÜ (WASAPI)
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

// --- YENİ EFEKT: PİKSEL AYRIŞMASI (DISINTEGRATION) ---
void DisintegrateEffect(HDC hdc, int durationMs) {
    DWORD start = GetTickCount();
    
    // Belirtilen süre boyunca çalış
    while (GetTickCount() - start < durationMs) {
        if (CheckExit()) exit(0);

        // Rastgele bir kaynak nokta seç
        int x = rand() % scrw;
        int y = rand() % scrh;

        // Rastgele bir hedef nokta seç
        int destX = rand() % scrw;
        int destY = rand() % scrh;

        // Çok küçük bir parçayı (2x2 piksel) alıp rastgele bir yere yapıştır
        // Bu "piksel piksel taşıma" görüntüsü verir
        BitBlt(hdc, destX, destY, 4, 4, hdc, x, y, SRCCOPY);
        
        // İşlemciyi yakmamak için minik bekleme (her döngüde değil, toplu hız)
        // Buraya Sleep koymuyoruz ki çok hızlı olsun, kaotik görünsün.
    }
}

// --- GEOMETRİK ANİMASYON ---
void PlayGeometrySequence(HDC hdc) {
    // 1. DAİRE
    int radius = 300;
    for (double angle = 0; angle < 2 * PI; angle += 0.1) {
        if (CheckExit()) exit(0);
        int x = centerX + (int)(radius * cos(angle));
        int y = centerY + (int)(radius * sin(angle));
        DrawIcon(hdc, x - 16, y - 16, LoadIcon(NULL, IDI_ERROR));
        Sleep(5); 
    }

    // 2. KARE
    int size = 350; 
    for (int x = centerX - size; x <= centerX + size; x += 40) {
        if (CheckExit()) exit(0);
        DrawIcon(hdc, x, centerY - size, LoadIcon(NULL, IDI_WARNING));
        DrawIcon(hdc, x, centerY + size, LoadIcon(NULL, IDI_WARNING));
        Sleep(5);
    }
    for (int y = centerY - size; y <= centerY + size; y += 40) {
        if (CheckExit()) exit(0);
        DrawIcon(hdc, centerX - size, y, LoadIcon(NULL, IDI_WARNING)); 
        DrawIcon(hdc, centerX + size, y, LoadIcon(NULL, IDI_WARNING)); 
        Sleep(5);
    }

    // 3. MERKEZ BİLGİ
    for (int x = centerX - 60; x <= centerX + 60; x+=60) {
        for (int y = centerY - 60; y <= centerY + 60; y+=60) {
             DrawIcon(hdc, x, y, LoadIcon(NULL, IDI_INFORMATION));
             Sleep(30);
        }
    }
}

// ANA RENDER (PIGSTEP DROP)
void RenderRhythmFrame(HDC hdc) {
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
    
    // Mouse Takip İzi
    POINT cursor;
    GetCursorPos(&cursor); 
    DrawIcon(hdc, cursor.x, cursor.y, LoadIcon(NULL, IDI_ERROR));
}

int main() {
    // 1. ADIM: GİZLEN
    GoStealth();
    Setup();
    HDC hdc = GetDC(0);

    // 2. ADIM: PİKSEL AYRIŞMASI (5 Saniye)
    // Ekran "erimeye" başlar
    DisintegrateEffect(hdc, 5000);

    // 3. ADIM: GEOMETRİK ŞOV
    PlayGeometrySequence(hdc);

    // 4. ADIM: DROP & SES
    SetMaxVolume(); 
    PlaySound(TEXT("pigstep.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    
    DWORD startTime = GetTickCount();
    DWORD lastBeatTime = 0;

    // SONSUZ DÖNGÜ
    while (true) {
        if (CheckExit()) break;

        DWORD currentTime = GetTickCount();
        DWORD elapsedTime = currentTime - startTime;

        if (elapsedTime - lastBeatTime >= BEAT_INTERVAL) {
            lastBeatTime += (DWORD)BEAT_INTERVAL;
            beatCounter++;

            // Heavy Beat (Her 2 vuruşta bir)
            if (beatCounter % 2 == 0) {
                intensity = 1.0; 
            }
        }

        RenderRhythmFrame(hdc);

        intensity *= 0.96; 
        if (intensity < 0.01) intensity = 0;

        Sleep(30); 
    }

    ReleaseDC(0, hdc);
    PlaySound(NULL, 0, 0);
    return 0;
}
