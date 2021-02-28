#pragma once
#include "GameTimer.h"

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

		int									Run();

	private:
		std::unique_ptr<Window>				mpWindow;
		std::unique_ptr<IRenderer>			mpRenderer;

		GameTimer							mGameTimer;
		int									mFixUpdateFrequency;
		double								mFixUpdatePeriod;

		bool								mIsGameRunning = false;

		void								PrivStart();
		void								PrivFixUpdate();
		void								PrivUpdate(const double aDeltaTime);
		void								PrivRender(const double aDeltaTime);

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
