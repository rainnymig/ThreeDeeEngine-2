#pragma once
#include "common/IRenderer.h"

namespace tde
{

	class SimpleMeshRenderer : public IRenderable
	{
	public:
		virtual void						Render(
												const double aDeltaTime) override;
	};
}