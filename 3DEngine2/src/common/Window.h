#pragma once

namespace tde
{
	class Window : ConstructorTagHelper
	{
	public:

		static bool				RegisterWindow(const WNDCLASSEX* apWndClassEx);
		static					std::unique_ptr<Window> MakeWindow(DWORD aExStyle, LPCWSTR aClassName, LPCWSTR aWindowName, DWORD aStyle,
								 int aX, int aY, int aWidth, int aHeight,
								 HWND aWndParent, HMENU aMenu, HINSTANCE ahInstance, LPVOID apParam);
		
								Window(ConstructorTag aConstructorTag, HWND aWindowHandle);

								~Window();
		
		HWND					GetWindowHandle() const;
		void					Show();
		void					Minimize();
		void					Maximize();
		void					ToggleFullscreen();

	private:

		HWND					mWindowHandle;
		bool					mIsFullscreen = false;

	};
}
