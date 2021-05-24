#include "pch.h"
#include "common/Window.h"

namespace tde
{
	Window::Window(
		ConstructorTag aConstructorTag, 
		HWND aWindowHandle)
		: mWindowHandle(aWindowHandle)
	{}

	bool Window::RegisterWindow(const WNDCLASSEX* apWndClassEx)
	{
		if (!RegisterClassExW(apWndClassEx))
		{
			return false;
		}
		return true;
	}

	std::unique_ptr<Window> Window::MakeWindow(
		DWORD aExStyle, 
		LPCWSTR aClassName, 
		LPCWSTR aWindowName, 
		DWORD aStyle,
		int aX, 
		int aY, 
		int aWidth, 
		int aHeight, 
		HWND aWndParent, 
		HMENU aMenu, 
		HINSTANCE ahInstance, 
		LPVOID apParam)
	{
		HWND hwnd = CreateWindowEx(aExStyle, aClassName, aWindowName, aStyle, aX, aY, aWidth, aHeight, aWndParent, aMenu, ahInstance, apParam);
		if (!hwnd)
		{
			throw std::runtime_error("failed to create window");
		}
		return std::make_unique<Window>(ConstructorTag(), hwnd);
	}

	Window::~Window()
	{
		DestroyWindow(mWindowHandle);
	}

	HWND Window::GetWindowHandle() const
	{
		return mWindowHandle;
	}

	void Window::Show()
	{
		ShowWindow(mWindowHandle, SW_SHOWNORMAL);
	}

	void Window::Minimize()
	{
		ShowWindow(mWindowHandle, SW_SHOWMINIMIZED);
	}

	void Window::Maximize()
	{
		ShowWindow(mWindowHandle, SW_SHOWMAXIMIZED);
	}

	void Window::ToggleFullscreen()
	{
		if (mIsFullscreen)
		{
			SetWindowLongPtr(mWindowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			SetWindowLongPtr(mWindowHandle, GWL_EXSTYLE, 0);

			int width = 800;
			int height = 600;

			ShowWindow(mWindowHandle, SW_SHOWNORMAL);

			SetWindowPos(mWindowHandle, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
		else
		{
			SetWindowLongPtr(mWindowHandle, GWL_STYLE, 0);
			SetWindowLongPtr(mWindowHandle, GWL_EXSTYLE, WS_EX_TOPMOST);

			SetWindowPos(mWindowHandle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			ShowWindow(mWindowHandle, SW_SHOWMAXIMIZED);
		}
		mIsFullscreen = !mIsFullscreen;
	}
}