#pragma once

namespace tde
{
	class IRenderer
	{
	public:
		virtual void Render(const double aDeltaTime) = 0;
		virtual void OnWindowSizeChanged(int aWidth, int aHeight) = 0;
	};

	class IRenderable
	{
	public:
		virtual void Render(const double aDeltaTime) = 0;
	};
}
