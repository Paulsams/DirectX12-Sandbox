#define WIN32_LEAN_AND_MEAN
#include "CubeRenderer.h"

#include <Windows.h>
#include <Shlwapi.h>

#include <Application.h>

#include <dxgidebug.h>

void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    int retCode = 0;

    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandleW(NULL);
    if ( GetModuleFileNameW(hModule, path, MAX_PATH) > 0 )
    {
        PathRemoveFileSpecW(path);
        SetCurrentDirectoryW(path);
    }

    FILE* fp;
    AllocConsole();
    freopen_s(&fp, "CONIN$", "r", stdin);
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    
    Application::Create(hInstance);
    {
        std::shared_ptr<CubeRenderer> demo = std::make_shared<CubeRenderer>(L"Learning DirectX 12 - Lesson 2", 1280, 720);
        retCode = Application::Get().Run(demo);
    }
    Application::Destroy();
    
    atexit(&ReportLiveObjects);

    return retCode;
}