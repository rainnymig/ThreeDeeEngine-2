#include "pch.h"
#include "Game.h"
#include "common/Configuration.h"
#include "common/Window.h"

using namespace DirectX;
using namespace tde;

#ifdef __clang__
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif

#pragma warning(disable : 4061)

namespace
{
    std::unique_ptr<Game> gGame;
}

//LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // load configuration
    Configuration::Initialize();
    Configuration::GetInstance()->LoadCfgFile("default.cfg");

    if (!XMVerifyCPUSupport())
        return 1;

    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;

    //gGame = std::make_unique<GameDR>();
    gGame = Game::MakeGame(hInstance);

    if (!gGame)
    {
        OutputDebugString(L"ERROR: fail to make game\n");
        return -1;
    }

    int gameReturnCode = gGame->Run();

    gGame.reset();

    CoUninitialize();

    return gameReturnCode;
}
