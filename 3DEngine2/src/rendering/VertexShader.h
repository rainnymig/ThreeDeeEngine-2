#pragma once

namespace tde
{
	class VertexShader
	{
	public:
		VertexShader(LPCWSTR aVsPath, ID3D11Device* apDevice);
		VertexShader(VertexShader&& aOther) noexcept;
		VertexShader& operator=(VertexShader&& aOther) noexcept;
		VertexShader(const VertexShader& aOther) = delete;
		VertexShader& operator=(const VertexShader& aOther) = delete;
		virtual ~VertexShader();

		ID3D11VertexShader* GetVertexShader() const;
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mpVertexShader;
	};
}