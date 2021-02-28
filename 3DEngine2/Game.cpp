#include "pch.h"
#include "Game.h"
#include "Window.h"
#include "IRenderer.h"
#include "DirectX11Renderer.h"
#include "Configuration.h"

namespace tde
{
    const static UINT DEFAULT_WINDOW_WIDTH  = 800;
    const static UINT DEFAULT_WINDOW_HEIGHT = 600;

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
        UINT clientWidth = Configuration::GetInstance()->GetIntOrDefault("MainWindow.Width", DEFAULT_WINDOW_WIDTH);
        UINT clientHeight = Configuration::GetInstance()->GetIntOrDefault("MainWindow.Height", DEFAULT_WINDOW_HEIGHT);

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
        std::unique_ptr<IRenderer> pRenderer = DirectX11Renderer::CreateDX11Renderer(pWindow->GetWindowHandle(), cliendRect);
        if (!pRenderer)
        {
            return nullptr;
        }

        //  create the Game object
		std::unique_ptr<Game> pGame = std::make_unique<Game>(ConstructorTag());
        pGame->mWindow = std::move(pWindow);
        pGame->mRenderer = std::move(pRenderer);

        //  save the pointer to the Game object so that you can use its members in WndProc
        SetWindowLongPtr(pWindow->GetWindowHandle(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pGame.get()));

        pGame->mWindow->Show();

		return pGame;
	}

	Game::Game(
		ConstructorTag aConstructorTag)
	{
	}

	Game::~Game()
	{
	}

	void Game::Run()
	{
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
    }

    LRESULT Game::PrivWindProc(
		HWND hWnd, 
		UINT message, 
		WPARAM wParam, 
		LPARAM lParam)
	{
        static bool s_in_sizemove = false;
        static bool s_in_suspend = false;
        static bool s_minimized = false;
        static bool s_fullscreen = false;
        // TODO: Set s_fullscreen to true if defaulting to fullscreen.

        auto game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (message)
        {
        case WM_PAINT:
            if (s_in_sizemove)
            {
                //game->Tick();
            }
            else
            {
                PAINTSTRUCT ps;
                (void)BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
            }
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
                if (!s_minimized)
                {
                    s_minimized = true;
                    if (!s_in_suspend && game)
                    {
                        game->PrivOnSuspending();
                    }
                    s_in_suspend = true;
                }
            }
            else if (s_minimized)
            {
                s_minimized = false;
                if (s_in_suspend && game)
                {
                    game->PrivOnResuming();
                }
                s_in_suspend = false;
            }
            else if (!s_in_sizemove)
            {
                game->PrivOnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            }
            break;

        case WM_ENTERSIZEMOVE:
            s_in_sizemove = true;
            break;

        case WM_EXITSIZEMOVE:
            s_in_sizemove = false;
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
            }
            break;

        case WM_POWERBROADCAST:
            switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
                if (!s_in_suspend && game)
                {
                    game->PrivOnSuspending();
                }
                s_in_suspend = true;
                return TRUE;

            case PBT_APMRESUMESUSPEND:
                if (!s_minimized)
                {
                    if (s_in_suspend && game)
                    {
                        game->PrivOnResuming();
                    }
                    s_in_suspend = false;
                }
                return TRUE;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SYSKEYDOWN:
            if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
            {
                // Implements the classic ALT+ENTER fullscreen toggle
                if (s_fullscreen)
                {
                    SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                    int width = Configuration::GetInstance()->GetIntOrDefault("MainWindow.Width", DEFAULT_WINDOW_WIDTH);
                    int height = Configuration::GetInstance()->GetIntOrDefault("MainWindow.Height", DEFAULT_WINDOW_HEIGHT);

                    ShowWindow(hWnd, SW_SHOWNORMAL);

                    SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }
                else
                {
                    SetWindowLongPtr(hWnd, GWL_STYLE, 0);
                    SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                    SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
                }

                s_fullscreen = !s_fullscreen;
            }
            break;

        case WM_MENUCHAR:
            // A menu is active and the user presses a key that does not correspond
            // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
            return MAKELRESULT(0, MNC_CLOSE);
        }


		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
