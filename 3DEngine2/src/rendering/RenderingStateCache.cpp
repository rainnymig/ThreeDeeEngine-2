#include "pch.h"
#include "rendering/RenderingStateCache.h"

namespace tde
{
	void provideRenderingStateCaches()
	{
		if (!SamplerCacheLocator::Get())
		{
			SamplerCacheLocator::Provide(std::make_shared<BaseCache<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>>>());
		}

		if (!RasterizerStateCacheLocator::Get())
		{
			RasterizerStateCacheLocator::Provide(std::make_shared<BaseCache<std::string, Microsoft::WRL::ComPtr<ID3D11RasterizerState>>>());
		}

		if (!BlendStateCacheLocator::Get())
		{
			BlendStateCacheLocator::Provide(std::make_shared<BaseCache<std::string, Microsoft::WRL::ComPtr<ID3D11BlendState>>>());
		}

		if (!DepthStencilStateCacheLocator::Get())
		{
			DepthStencilStateCacheLocator::Provide(std::make_shared<BaseCache<std::string, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>>>());
		}
	}
}