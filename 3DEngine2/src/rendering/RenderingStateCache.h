#pragma once
#include "common/ServiceLocator.h"
#include "common/BaseCache.h"

namespace tde
{
	using ISamplerCache = ICache<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>>;
	using SamplerCacheLocator = ServiceLocator<ISamplerCache>;
	std::shared_ptr<ISamplerCache> SamplerCacheLocator::mpService = nullptr;

	using IRasterizerStateCache = ICache<std::string, Microsoft::WRL::ComPtr<ID3D11RasterizerState>>;
	using RasterizerStateCacheLocator = ServiceLocator<IRasterizerStateCache>;
	std::shared_ptr<IRasterizerStateCache> RasterizerStateCacheLocator::mpService = nullptr;

	using IBlendStateCache = ICache<std::string, Microsoft::WRL::ComPtr<ID3D11BlendState>>;
	using BlendStateCacheLocator = ServiceLocator<IBlendStateCache>;
	std::shared_ptr<IBlendStateCache> BlendStateCacheLocator::mpService = nullptr;

	using IDepthStencilStateCache = ICache<std::string, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>>;
	using DepthStencilStateCacheLocator = ServiceLocator<IDepthStencilStateCache>;
	std::shared_ptr<IDepthStencilStateCache> DepthStencilStateCacheLocator::mpService = nullptr;

	void provideRenderingStateCaches();
}
