#pragma once

namespace tde
{
	class PixelShader
	{
	public:
		PixelShader(LPCWSTR aPsPath, ID3D11Device* apDevice);
		PixelShader(PixelShader&& aOther) noexcept;
		PixelShader& operator=(PixelShader&& aOther) noexcept;
		PixelShader(const PixelShader& aOther) = delete;
		PixelShader& operator=(const PixelShader& aOther) = delete;
		virtual ~PixelShader();

		ID3D11PixelShader* GetPixelShader() const;
	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mpPixelShader;
	};
}