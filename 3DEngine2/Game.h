#pragma once

namespace tde
{
	class Window;
	class IRenderer;

	class Game : public ConstructorTagHelper
	{
	public:
		static std::unique_ptr<Game>		MakeGame(HINSTANCE ahInstance);

											Game(
												ConstructorTag			aConstructorTag);
											~Game();

		void								Run();

	private:
		std::unique_ptr<Window>				mWindow;
		std::unique_ptr<IRenderer>			mRenderer;

		void								PrivOnSuspending();
		void								PrivOnResuming();
		void								PrivOnSuspended();
		void								PrivOnResumed();
		void								PrivOnActivated();
		void								PrivOnDeactivated();
		void								PrivOnWindowMoved();
		void								PrivOnWindowSizeChanged(
												int aWidth, 
												int aHeight);

		static LRESULT CALLBACK				PrivWindProc(
												HWND hWnd, 
												UINT message, 
												WPARAM wParam, 
												LPARAM lParam);
	};
}
