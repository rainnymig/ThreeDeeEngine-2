#pragma once

namespace tde
{
	class StepTimer;

	class IRenderer
	{
	public:
		virtual void Render(const StepTimer& aTimer) = 0;
	};

	class IRenderable
	{
	public:
		virtual void Render(const StepTimer& aTimer) = 0;
	};
}
