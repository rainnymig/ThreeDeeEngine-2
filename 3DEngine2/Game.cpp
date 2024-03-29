#include "pch.h"
#include "Game.h"
#include "common/Window.h"
#include "common/DirectX11Renderer.h"
#include "common/Configuration.h"
#include "common/ServiceLocator.h"
#include "common/BaseCache.h"
#include "rendering/PixelShader.h"
#include "rendering/VertexShader.h"
#include "rendering/RenderingStateCache.h"

namespace tde
{
	std::unique_ptr<Game> tde::Game::MakeGame(HINSTANCE ahInstance)
	{
        static UINT classId = 1;

        std::wstring className(L"_3DEngine2WindowClass");
        className += std::to_wstring(classId);
        classId++;

        //  create the window class
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = &PrivWindProc;
        wcex.hInstance = ahInstance;
        wcex.hIcon = LoadIconW(ahInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszClassName = className.c_str();
        wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
        if (!Window::RegisterWindow(&wcex))
        {
            return nullptr;
        }
        UINT clientWidth = Configuration::GetInstance()->GetInt("MainWindow.Width");
        UINT clientHeight = Configuration::GetInstance()->GetInt("MainWindow.Height");

        RECT cliendRect = { 0, 0, static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
        AdjustWindowRect(&cliendRect, WS_OVERLAPPEDWINDOW, FALSE);

        //  create the window
        std::unique_ptr<Window> pWindow;
        try
        {
            pWindow = Window::MakeWindow(0, className.c_str(), L"3DEngine2",
                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
                cliendRect.right - cliendRect.left, cliendRect.bottom - cliendRect.top, nullptr, nullptr, ahInstance, nullptr);
        }
        catch (std::runtime_error e)
        {
            return nullptr;
        }
        
        //  create the Renderer
        std::unique_ptr<DirectX11Renderer> pRenderer = DirectX11Renderer::CreateDX11Renderer(pWindow->GetWindowHandle(), cliendRect);
        if (!pRenderer)
        {
            return nullptr;
        }

        //  create the Game object
		std::unique_ptr<Game> pGame = std::make_unique<Game>(ConstructorTag());
        pGame->mpWindow = std::move(pWindow);
        pGame->mpRenderer = std::move(pRenderer);

        //  create shader cache
        if (!VertexShaderCacheLocator::Get())
        {
            VertexShaderCacheLocator::Provide(std::make_shared<BaseCache<std::string, std::shared_ptr<VertexShader>>>());
        }
        if (!PixelShaderCacheLocator::Get())
        {
            PixelShaderCacheLocator::Provide(std::make_shared<BaseCache<std::string, std::shared_ptr<PixelShader>>>());
        }

        //  create rendering state caches
        provideRenderingStateCaches();


        //  save the pointer to the Game object so that you can use its members in WndProc
        SetWindowLongPtr(pGame->mpWindow->GetWindowHandle(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pGame.get()));

        pGame->mpWindow->Show();

		return pGame;
	}

	Game::Game(
		ConstructorTag aConstructorTag)
	{
	}

	Game::~Game()
	{
	}

	int Game::Run()
	{
        mFixUpdateFrequency = Configuration::GetInstance()->GetInt("Game.FixUpdateFrequency");
        mFixUpdatePeriod = 1.0 / mFixUpdateFrequency;

        const int maxFixUpdatesPerFrame = Configuration::GetInstance()->GetInt("Game.MaxFixUpdatesPerFrame");
     
        //  setup the all the things
        PrivStart();

        double accumulatedFixUpdateTime = 0.0;
        double deltaTime = 0.0;
        int fixUpdatesThisFrame = 0;
        mGameTimer.Reset();
        mGameTimer.Start();

        MSG msg = {};
        mIsGameRunning = true;
        while (mIsGameRunning)
        {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT)
                {
                    mIsGameRunning = false;
                }
            }

            mGameTimer.Tick();
            deltaTime = mGameTimer.GetDeltaTime();
            accumulatedFixUpdateTime += deltaTime;
            fixUpdatesThisFrame = 0;

            while (fixUpdatesThisFrame < maxFixUpdatesPerFrame && accumulatedFixUpdateTime > mFixUpdatePeriod)
            {
                accumulatedFixUpdateTime -= mFixUpdatePeriod;
                PrivFixUpdate();
                fixUpdatesThisFrame++;
            }

            PrivUpdate(deltaTime);
            PrivRender(deltaTime);
        }

        PrivDestroy();

        return static_cast<int>(msg.wParam);
	}

    void Game::PrivStart()
    {
        mpScene = std::make_unique<Scene>();
        mpScene->Init(mpRenderer->GetDevice(), mpWindow->GetWindowHandle());
    }

    void Game::PrivFixUpdate()
    {
        //  fix update
    }

    void Game::PrivUpdate(
        const double aDeltaTime)
    {
        mpScene->Update(mpRenderer->GetDevice(), aDeltaTime);
    }

    void Game::PrivRender(
        const double aDeltaTime)
    {
        mpRenderer->Clear();
        
        mpRenderer->SetRenderToOffscreenTarget();
        mpScene->Render(mpRenderer->GetDevice(), mpRenderer->GetImmediateContext(), aDeltaTime);
        mpRenderer->SetRenderToOnscreenTarget();
        mpScene->PostProcess(mpRenderer->GetImmediateContext(), mpRenderer->GetRawRenderTargetSRV(), mpRenderer->GetDepthStencilSRV(), aDeltaTime);

        mpRenderer->Present();
    }

    void Game::PrivDestroy()
    {
        mpScene->Destroy();
    }

    void Game::PrivOnSuspending()
    {
    }

    void Game::PrivOnResuming()
    {
    }

    void Game::PrivOnSuspended()
    {
    }

    void Game::PrivOnResumed()
    {
    }

    void Game::PrivOnActivated()
    {
    }

    void Game::PrivOnDeactivated()
    {
    }

    void Game::PrivOnWindowMoved()
    {
    }

    void Game::PrivOnWindowSizeChanged(int aWidth, int aHeight)
    {
        mpRenderer->OnWindowSizeChanged(aWidth, aHeight);
        if (mpScene)
        {
            mpScene->OnScreenSizeChange(aWidth, aHeight);
        }
    }

    LRESULT Game::PrivWindProc(
		HWND hWnd, 
		UINT message, 
		WPARAM wParam, 
		LPARAM lParam)
	{
        static bool isSizemoving = false;
        static bool isSuspending = false;
        static bool isMinimized = false;
        static bool isFullscreen = false;
        // TODO: Set isFullscreen to true if defaulting to fullscreen.

        auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (message)
        {
        case WM_PAINT:
            PAINTSTRUCT ps;
            (void)BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_MOVE:
            if (game)
            {
                game->PrivOnWindowMoved();
            }
            break;

        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                if (!isMinimized)
                {
                    isMinimized = true;
                    if (!isSuspending && game)
                    {
                        game->PrivOnSuspending();
                    }
                    isSuspending = true;
                }
            }
            else if (isMinimized)
            {
                isMinimized = false;
                if (isSuspending && game)
                {
                    game->PrivOnResuming();
                }
                isSuspending = false;
            }
            else if (!isSizemoving)
            {
                game->PrivOnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            }
            break;

        case WM_ENTERSIZEMOVE:
            isSizemoving = true;
            break;

        case WM_EXITSIZEMOVE:
            isSizemoving = false;
            if (game)
            {
                RECT rc;
                GetClientRect(hWnd, &rc);

                game->PrivOnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            }
            break;

        case WM_GETMINMAXINFO:
            if (lParam)
            {
                auto info = reinterpret_cast<MINMAXINFO*>(lParam);
                info->ptMinTrackSize.x = 320;
                info->ptMinTrackSize.y = 200;
            }
            break;

        case WM_ACTIVATEAPP:
            if (game)
            {
                if (wParam)
                {
                    game->PrivOnActivated();
                }
                else
                {
                    game->PrivOnDeactivated();
                }
                DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
                DirectX::Mouse::ProcessMessage(message, wParam, lParam);
            }
            break;

        case WM_POWERBROADCAST:
            switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
                if (!isSuspending && game)
                {
                    game->PrivOnSuspending();
                }
                isSuspending = true;
                return TRUE;

            case PBT_APMRESUMESUSPEND:
                if (!isMinimized)
                {
                    if (isSuspending && game)
                    {
                        game->PrivOnResuming();
                    }
                    isSuspending = false;
                }
                return TRUE;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
            DirectX::Mouse::ProcessMessage(message, wParam, lParam);
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
            break;

        //case WM_SYSKEYDOWN:
        //    if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        //    {
        //        // Implements the classic ALT+ENTER fullscreen toggle
        //        if (isFullscreen)
        //        {
        //            SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        //            SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);
        //            int width = Configuration::GetInstance()->GetInt("MainWindow.Width");
        //            int height = Configuration::GetInstance()->GetInt("MainWindow.Height");
        //            ShowWindow(hWnd, SW_SHOWNORMAL);
        //            SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        //        }
        //        else
        //        {
        //            SetWindowLongPtr(hWnd, GWL_STYLE, 0);
        //            SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
        //            SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        //            ShowWindow(hWnd, SW_SHOWMAXIMIZED);
        //        }
        //        isFullscreen = !isFullscreen;
        //    }
        //    break;

        case WM_MENUCHAR:
            //  A menu is active and the user presses a key that does not correspond
            //  to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
            return MAKELRESULT(0, MNC_CLOSE);
        }


		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
