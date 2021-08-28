#pragma once
#include "common/ServiceLocator.h"
#include "common/BaseCache.h"

namespace tde
{
	class VertexShader
	{
	public:
		VertexShader(LPCWSTR aVsPath, const D3D11_INPUT_ELEMENT_DESC* apLayout, UINT aLayoutElementCount, ID3D11Device* apDevice);
		VertexShader(VertexShader&& aOther) noexcept;
		VertexShader& operator=(VertexShader&& aOther) noexcept;
		VertexShader(const VertexShader& aOther) = delete;
		VertexShader& operator=(const VertexShader& aOther) = delete;
		virtual ~VertexShader();

		ID3D11VertexShader* GetVertexShader() const;
		void SetInputLayout(ID3D11DeviceContext* apContext) const;

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mpVertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mpInputLayout;
	};

	using IVertexShaderCache = ICache<std::string, std::shared_ptr<VertexShader>>;
	using VertexShaderCacheLocator = ServiceLocator<IVertexShaderCache>;
	std::shared_ptr<IVertexShaderCache> VertexShaderCacheLocator::mpService = nullptr;
}