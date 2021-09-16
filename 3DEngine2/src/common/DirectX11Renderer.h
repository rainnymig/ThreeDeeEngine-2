#pragma once

#include "IRenderer.h"

namespace tde
{
	class Window;

	class DirectX11Renderer final 
		: public ConstructorTagHelper
		, public IRenderer
	{
	public:

		static const UINT OPTION_FLIP_PRESENT		= 0x1;
		static const UINT OPTION_ALLOW_TEARING		= 0x2;
		static const UINT OPTION_ENABLE_HDR			= 0x4;

										//	Note: the back buffer format is default to BGRA format
										//		  because this has slightly better performance when "flipping"
										//		  https://gamedev.stackexchange.com/questions/65743/directx11-swap-chain-rgba-vs-bgra-format
		static std::unique_ptr<DirectX11Renderer> CreateDX11Renderer(
											HWND				aWindowHandle,
											RECT				aClientRect,
											DXGI_FORMAT			aBackBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
											DXGI_FORMAT			aDepthStencilBufferFormat = DXGI_FORMAT_R24G8_TYPELESS,
											UINT				aBackBufferCount = 2,
											D3D_FEATURE_LEVEL	aMinFeatureLevel = D3D_FEATURE_LEVEL_11_0,
											UINT				aOptions = OPTION_FLIP_PRESENT);

										DirectX11Renderer(
											ConstructorTag		aConstructorTag,
											HWND				aWindowHandle,
											RECT				aClientRect,
											DXGI_FORMAT			aBackBufferFormat,
											DXGI_FORMAT			aDepthStencilBufferFormat,
											UINT				aBackBufferCount,
											D3D_FEATURE_LEVEL	aMinFeatureLevel,
											UINT				aOptions);

										~DirectX11Renderer() = default;

		virtual void					Render(
											const double		aDeltaTime) override;
		void							Clear();
		void							SetRenderToOffscreenTarget();
		void							SetRenderToOnscreenTarget();
		void							Present();
		void							AddToRenderList(
											std::shared_ptr<IRenderable>	apRenderable);
		void							OnWindowSizeChanged(int aWidth, int aHeight) override;

		auto							GetFactory() const noexcept { return mpDxgiFactory.Get(); }
		auto							GetDevice() const noexcept { return mpD3d11Device.Get(); }
		auto							GetImmediateContext() const noexcept { return mpD3d11ImmediateContext.Get(); }
		auto							GetSwapChain() const noexcept { return mpDxgiSwapChain.Get(); }
		ID3D11Texture2D*				GetRenderTarget() const noexcept { return mpRenderTarget.Get(); }
		ID3D11RenderTargetView*			GetRenderTargetView() const noexcept { return mpRenderTargetView.Get(); }
		ID3D11Texture2D*				GetRawRenderTarget() const noexcept { return mpRawRenderTarget.Get(); }
		ID3D11RenderTargetView*			GetRawRenderTargetView() const noexcept { return mpRawRenderTargetView.Get(); }
		ID3D11ShaderResourceView*		GetRawRenderTargetSRV() const noexcept { return mpRawRenderTargetSRV.Get(); }
		ID3D11Texture2D*				GetDepthStencil() const noexcept { return mpDepthStencil.Get(); }
		ID3D11DepthStencilView*			GetDepthStencilView() const noexcept { return mpDepthStencilView.Get(); }
		ID3D11ShaderResourceView*		GetDepthStencilSRV() const noexcept { return mpDepthStencilSRV.Get(); }
		D3D11_VIEWPORT					GetViewport() const noexcept { return mViewport; }
		HWND							GetWindowHandle() const noexcept { return mWindowHandle; }
		RECT							GetWindowRect() const noexcept { return mWindowRect; }
		D3D_FEATURE_LEVEL				GetMinFeatureLevel() const noexcept { return mMinFeatureLevel; }
		DXGI_COLOR_SPACE_TYPE			GetColorSpace() const noexcept { return mColorSpace; }
		DXGI_FORMAT						GetBackBufferFormat() const noexcept { return mBackBufferFormat; }
		DXGI_FORMAT						GetDepthStencilFormat() const noexcept { return mDepthStencilFormat; }
		UINT							GetBackBufferCount() const noexcept { return mBackBufferCount; }
		UINT							GetOptions() const noexcept { return mOptions; }

	private:

		void							PrivCreateDeviceResources();
		void							PrivCreateWindowSizeDependentResources();

		HRESULT							PrivCreateFactory();
		void							PrivCheckOptions();
		HRESULT							PrivGetHardwareAdapter(
											IDXGIAdapter1** appAdapter);
		HRESULT							PrivUpdateColorSpace();
		void							PrivHandleDeviceLost();


		std::list<std::shared_ptr<IRenderable>>							mRenderableList;

		Microsoft::WRL::ComPtr<IDXGIFactory2>							mpDxgiFactory;
		Microsoft::WRL::ComPtr<ID3D11Device1>							mpD3d11Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1>					mpD3d11ImmediateContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>							mpDxgiSwapChain;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>							mpRawRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>					mpRawRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>				mpRawRenderTargetSRV;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>							mpRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>					mpRenderTargetView;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>							mpDepthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>					mpDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>				mpDepthStencilSRV;
		
		D3D11_VIEWPORT													mViewport;

		HWND															mWindowHandle;
		RECT															mWindowRect;
		D3D_FEATURE_LEVEL												mMinFeatureLevel;

		DXGI_COLOR_SPACE_TYPE											mColorSpace;

		DXGI_FORMAT														mBackBufferFormat;
		DXGI_FORMAT														mDepthStencilFormat;
		UINT															mBackBufferCount;
		UINT															mOptions;

	};
}
