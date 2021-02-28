#include "pch.h"
#include "DirectX11Renderer.h"

#include "StepTimer.h"

using Microsoft::WRL::ComPtr;

namespace tde
{
#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable() noexcept
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			nullptr,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
		);

		return SUCCEEDED(hr);
	}
#endif

	inline DXGI_FORMAT NoSRGB(
		DXGI_FORMAT aFormat) noexcept
	{
		switch (aFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
		default:                                return aFormat;
		}
	}

	std::unique_ptr<DirectX11Renderer> DirectX11Renderer::CreateDX11Renderer(
		HWND aWindowHandle,
		RECT aClientRect,
		DXGI_FORMAT aBackBufferFormat, 
		DXGI_FORMAT aDepthStencilBufferFormat, 
		UINT aBackBufferCount, 
		D3D_FEATURE_LEVEL aMinFeatureLevel, 
		UINT aOptions)
	{
		std::unique_ptr<DirectX11Renderer> pRenderer = std::make_unique<DirectX11Renderer>(
																					ConstructorTag(), 
																					aWindowHandle,
																					aClientRect,
																					aBackBufferFormat, 
																					aDepthStencilBufferFormat, 
																					aBackBufferCount, 
																					aMinFeatureLevel, 
																					aOptions);

		try
		{
			pRenderer->PrivCreateDeviceResources();
			pRenderer->PrivCreateWindowSizeDependentResources();
		}
		catch (std::exception e)
		{
			return nullptr;
		}

		return pRenderer;
	}

	DirectX11Renderer::DirectX11Renderer(
		ConstructorTag aConstructorTag,
		HWND aWindowHandle,
		RECT aClientRect,
		DXGI_FORMAT aBackBufferFormat, 
		DXGI_FORMAT aDepthStencilBufferFormat, 
		UINT aBackBufferCount, 
		D3D_FEATURE_LEVEL aMinFeatureLevel, 
		UINT aOptions)
		: mBackBufferFormat(aBackBufferFormat)
		, mDepthStencilFormat(aDepthStencilBufferFormat)
		, mBackBufferCount(aBackBufferCount)
		, mMinFeatureLevel(aMinFeatureLevel)
		, mOptions(aOptions)
		, mViewport()
		, mWindowHandle(aWindowHandle)
		, mWindowRect(aClientRect)
		, mColorSpace()
	{
	}

	void DirectX11Renderer::Render(const StepTimer& aTimer)
	{
	}

	void DirectX11Renderer::Present()
	{
		HRESULT hr = E_FAIL;
		if (mOptions & OPTION_ALLOW_TEARING)
		{
			//	Recommended to always use tearing if supported when using a sync interval of 0.
			hr = mpDxgiSwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
		}
		else
		{
			//	The first argument instructs DXGI to block until VSync, putting the application
			//	to sleep until the next VSync. This ensures we don't waste any cycles rendering
			//	frames that will never be displayed to the screen.
			hr = mpDxgiSwapChain->Present(1, 0);
		}

		//	Discard the contents of the render target.
		//	This is a valid operation only when the existing contents will be entirely
		//	overwritten. If dirty or scroll rects are used, this call should be removed.
		mpD3d11ImmediateContext->DiscardView(mpRenderTargetView.Get());

		if (mpDepthStencilView)
		{
			//	Discard the contents of the depth stencil.
			mpD3d11ImmediateContext->DiscardView(mpDepthStencilView.Get());
		}

		//	If the device was removed either by a disconnection or a driver upgrade, we
		//	must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
#ifdef _DEBUG
			char buff[64] = {};
			sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
				static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? mpD3d11Device->GetDeviceRemovedReason() : hr));
			OutputDebugStringA(buff);
#endif
			PrivHandleDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr);

			if (!mpDxgiFactory->IsCurrent())
			{
				// Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
				PrivCreateFactory();
			}
		}
	}

	void DirectX11Renderer::AddToRenderList(std::shared_ptr<IRenderable> apRenderable)
	{
		mRenderableList.emplace_back(apRenderable);
	}

	bool DirectX11Renderer::OnWindowSizeChanged(int width, int height)
	{
		RECT windowRect;
		windowRect.left = 0;
		windowRect.top = 0;
		windowRect.right = width;
		windowRect.bottom = height;

		if (windowRect == mWindowRect)
		{
			PrivUpdateColorSpace();

			return false;
		}

		mWindowRect = windowRect;
		PrivCreateWindowSizeDependentResources();
		return true;
	}

	void DirectX11Renderer::PrivCreateDeviceResources()
	{
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
		if (SdkLayersAvailable())
		{
			// If the project is in a debug build, enable debugging via SDK Layers with this flag.
			creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}
		else
		{
			OutputDebugString(L"WARNING: Direct3D Debug Device is not available\n");
		}
#endif

		ThrowIfFailed(PrivCreateFactory());

		PrivCheckOptions();

		// Determine DirectX hardware feature levels this app will support.
		static const D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};
		UINT featureLevelCount = 0;
		for (; featureLevelCount < _countof(featureLevels); ++featureLevelCount)
		{
			if (featureLevels[featureLevelCount] < mMinFeatureLevel)
				break;
		}
		if (!featureLevelCount)
		{
#ifdef _DEBUG
			OutputDebugString(L"ERROR: minFeatureLevel too high");
#endif
			throw std::exception("minFeatureLevel too high");
		}

		ComPtr<IDXGIAdapter1> adapter;

		ThrowIfFailed(PrivGetHardwareAdapter(adapter.GetAddressOf()));

		// Create the Direct3D 11 API device object and a corresponding context.
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;

		if (adapter)
		{
			ThrowIfFailed(D3D11CreateDevice(
				adapter.Get(),
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				creationFlags,
				featureLevels,
				featureLevelCount,
				D3D11_SDK_VERSION,
				device.ReleaseAndGetAddressOf(),
				&mMinFeatureLevel,
				context.ReleaseAndGetAddressOf()
			));
		}
		else
		{
			throw std::exception("No Direct3D hardware device found");
		}

#ifndef NDEBUG
		ComPtr<ID3D11Debug> d3dDebug;
		if (SUCCEEDED(device.As(&d3dDebug)))
		{
			ComPtr<ID3D11InfoQueue> d3dInfoQueue;
			if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
			{
#ifdef _DEBUG
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
				D3D11_MESSAGE_ID hide[] =
				{
					D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				};
				D3D11_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);
			}
		}
#endif

		ThrowIfFailed(device.As(&mpD3d11Device));
		ThrowIfFailed(context.As(&mpD3d11ImmediateContext));
	}

	void DirectX11Renderer::PrivCreateWindowSizeDependentResources()
	{
		//	Clear the previous window size specific context.
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		mpD3d11ImmediateContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
		mpRenderTargetView.Reset();
		mpDepthStencilView.Reset();
		mpRenderTarget.Reset();
		mpDepthStencil.Reset();
		mpD3d11ImmediateContext->Flush();

		const UINT backBufferWidth = std::max<UINT>(static_cast<UINT>(mWindowRect.right - mWindowRect.left), 1u);
		const UINT backBufferHeight = std::max<UINT>(static_cast<UINT>(mWindowRect.bottom - mWindowRect.top), 1u);
		const DXGI_FORMAT backBufferFormat = (mOptions & (OPTION_FLIP_PRESENT | OPTION_ALLOW_TEARING | OPTION_ENABLE_HDR)) 
												? NoSRGB(mBackBufferFormat) 
												: mBackBufferFormat;

		if (mpDxgiSwapChain)
		{
			//	If the swap chain already exists, resize it.
			HRESULT hr = mpDxgiSwapChain->ResizeBuffers(
				mBackBufferCount, 
				backBufferWidth, 
				backBufferHeight, 
				backBufferFormat, 
				(mOptions & OPTION_ALLOW_TEARING) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
#ifdef _DEBUG
				char buff[64] = {};
				sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
					static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? mpD3d11Device->GetDeviceRemovedReason() : hr));
				OutputDebugStringA(buff);
#endif
				//	If the device was removed for any reason, a new device and swap chain will need to be created.
				PrivHandleDeviceLost();

				//	Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
				//	and correctly set up the new device.
				return;
			}
			else
			{
				ThrowIfFailed(hr);
			}
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.Width = backBufferWidth;
			swapChainDesc.Height = backBufferHeight;
			swapChainDesc.Format = backBufferFormat;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = mBackBufferCount;
			swapChainDesc.SampleDesc.Count = 1;		//	Note: no MSAA with flip present, need to do that somewhere else
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesc.SwapEffect = (mOptions & (OPTION_FLIP_PRESENT | OPTION_ALLOW_TEARING | OPTION_ENABLE_HDR)) 
											? DXGI_SWAP_EFFECT_FLIP_DISCARD 
											: DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			swapChainDesc.Flags = (mOptions & OPTION_ALLOW_TEARING) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;
			
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
			fsSwapChainDesc.Windowed = TRUE;

			ThrowIfFailed(mpDxgiFactory->CreateSwapChainForHwnd(
												mpD3d11Device.Get(),
												mWindowHandle,
												&swapChainDesc,
												&fsSwapChainDesc,
												nullptr,
												mpDxgiSwapChain.ReleaseAndGetAddressOf()));

			//	This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
			ThrowIfFailed(mpDxgiFactory->MakeWindowAssociation(mWindowHandle, DXGI_MWA_NO_ALT_ENTER));
		}

		PrivUpdateColorSpace();
		
		//	Create a render target view of the swap chain back buffer.
		ThrowIfFailed(mpDxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(mpRenderTarget.ReleaseAndGetAddressOf())));
		CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, mBackBufferFormat);
		ThrowIfFailed(mpD3d11Device->CreateRenderTargetView(mpRenderTarget.Get(), &rtvDesc, mpRenderTargetView.ReleaseAndGetAddressOf()));

		if (mDepthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			CD3D11_TEXTURE2D_DESC dsDesc(
				mDepthStencilFormat, 
				backBufferWidth, 
				backBufferHeight, 
				1, //	This depth stencil view has only one texture.
				1, //	Use a single mipmap level.
				D3D11_BIND_DEPTH_STENCIL);
			ThrowIfFailed(mpD3d11Device->CreateTexture2D(&dsDesc, nullptr, mpDepthStencil.ReleaseAndGetAddressOf()));
			CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
			ThrowIfFailed(mpD3d11Device->CreateDepthStencilView(mpDepthStencil.Get(), &dsvDesc, mpDepthStencilView.ReleaseAndGetAddressOf()));
		}

		//	Set the 3D rendering viewport to target the entire window.
		mViewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(backBufferWidth), static_cast<float>(backBufferHeight));
			
	}

	HRESULT DirectX11Renderer::PrivCreateFactory()
	{
		HRESULT hr;

#if defined(_DEBUG) && (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		bool debugDXGI = false;
		{
			ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
			{
				debugDXGI = true;

				hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(mpDxgiFactory.ReleaseAndGetAddressOf()));

				RETURN_IF_FAILED(hr);

				dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
				dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

				DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
				{
					80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
				};
				DXGI_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
			}
		}

		if (!debugDXGI)
#endif

		hr = CreateDXGIFactory1(IID_PPV_ARGS(mpDxgiFactory.ReleaseAndGetAddressOf()));
		return hr;
	}

	void DirectX11Renderer::PrivCheckOptions()
	{
		//	
		if (!mpDxgiFactory)
		{
			return;
		}

		//	Determines whether tearing support is available for fullscreen borderless windows.
		if (mOptions & OPTION_ALLOW_TEARING)
		{
			BOOL allowTearing = FALSE;

			ComPtr<IDXGIFactory5> factory5;
			HRESULT hr = mpDxgiFactory.As(&factory5);
			if (SUCCEEDED(hr))
			{
				hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
			}
			if (FAILED(hr) && !allowTearing)
			{
				mOptions &= ~OPTION_ALLOW_TEARING;
#ifdef _DEBUG
				OutputDebugString(L"WARNING: Variable refresh rate displays not supported");
#endif
			}
		}

		//	Disable HDR if we are on an OS that can't support FLIP swap effects
		if (mOptions & OPTION_ENABLE_HDR)
		{
			ComPtr<IDXGIFactory5> factory5;
			HRESULT hr = mpDxgiFactory.As(&factory5);
			if (FAILED(hr))
			{
				mOptions &= ~OPTION_ENABLE_HDR;
#ifdef _DEBUG
				OutputDebugString(L"WARNING: HDR swap chains not supported");
#endif
			}
		}

		//	Disable FLIP if not on a supporting OS
		if (mOptions & OPTION_FLIP_PRESENT)
		{
			ComPtr<IDXGIFactory4> factory4;
			HRESULT hr = mpDxgiFactory.As(&factory4);
			if (FAILED(hr))
			{
				mOptions &= ~OPTION_FLIP_PRESENT;
#ifdef _DEBUG
				OutputDebugString(L"INFO: Flip swap effects not supported");
#endif
			}
		}
	}

	HRESULT DirectX11Renderer::PrivGetHardwareAdapter(
		IDXGIAdapter1** appAdapter)
	{
		HRESULT hr = E_FAIL;

		if (!mpDxgiFactory)
		{
			return hr;
		}

		*appAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
		ComPtr<IDXGIFactory6> factory6;
		hr = mpDxgiFactory.As(&factory6);
		if (SUCCEEDED(hr))
		{
			for (UINT adapterIndex = 0;
				SUCCEEDED(factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
					IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
				adapterIndex++)
			{
				DXGI_ADAPTER_DESC1 desc;
				RETURN_IF_FAILED(adapter->GetDesc1(&desc));

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					continue;
				}

#ifdef _DEBUG
				wchar_t buff[256] = {};
				swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
				OutputDebugStringW(buff);
#endif

				break;
			}
		}
#endif

		if (!adapter)
		{
			for (UINT adapterIndex = 0;
				SUCCEEDED(mpDxgiFactory->EnumAdapters1(
					adapterIndex,
					adapter.ReleaseAndGetAddressOf()));
				adapterIndex++)
			{
				DXGI_ADAPTER_DESC1 desc;
				RETURN_IF_FAILED(adapter->GetDesc1(&desc));

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					continue;
				}

#ifdef _DEBUG
				wchar_t buff[256] = {};
				swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
				OutputDebugStringW(buff);
#endif

				break;
			}
		}

		*appAdapter = adapter.Detach();

		return hr;
	}

	HRESULT DirectX11Renderer::PrivUpdateColorSpace()
	{
		HRESULT hr = E_FAIL;

		DXGI_COLOR_SPACE_TYPE colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

		bool isDisplayHDR10 = false;

#if defined(NTDDI_WIN10_RS2)
		if (mpDxgiSwapChain)
		{
			ComPtr<IDXGIOutput> output;
			hr = mpDxgiSwapChain->GetContainingOutput(output.GetAddressOf());
			if (SUCCEEDED(hr))
			{
				ComPtr<IDXGIOutput6> output6;
				if (SUCCEEDED(output.As(&output6)))
				{
					DXGI_OUTPUT_DESC1 desc;
					hr = output6->GetDesc1(&desc);
					RETURN_IF_FAILED(hr);

					if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
					{
						// Display output is HDR10.
						isDisplayHDR10 = true;
					}
				}
			}
		}
#endif

		if ((mOptions & OPTION_ENABLE_HDR) && isDisplayHDR10)
		{
			switch (mBackBufferFormat)
			{
			case DXGI_FORMAT_R10G10B10A2_UNORM:
				// The application creates the HDR10 signal.
				colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
				break;

			case DXGI_FORMAT_R16G16B16A16_FLOAT:
				// The system creates the HDR10 signal; application uses linear values.
				colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
				break;

			default:
				break;
			}
		}

		mColorSpace = colorSpace;

		ComPtr<IDXGISwapChain3> swapChain3;
		hr = mpDxgiSwapChain.As(&swapChain3);
		if (SUCCEEDED(hr))
		{
			UINT colorSpaceSupport = 0;
			hr = swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport);
			if (SUCCEEDED(hr)
				&& (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
			{
				hr = swapChain3->SetColorSpace1(colorSpace);
				RETURN_IF_FAILED(hr);
			}
		}

		return hr;
	}

	void DirectX11Renderer::PrivHandleDeviceLost()
	{
		mpRenderTargetView.Reset();
		mpDepthStencilView.Reset();
		mpRenderTarget.Reset();
		mpDepthStencil.Reset();
		mpDxgiSwapChain.Reset();
		mpD3d11ImmediateContext.Reset();

#ifdef _DEBUG
		{
			ComPtr<ID3D11Debug> d3dDebug;
			if (SUCCEEDED(mpD3d11Device.As(&d3dDebug)))
			{
				d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
			}
		}
#endif

		mpD3d11Device.Reset();
		mpDxgiFactory.Reset();

		PrivCreateDeviceResources();
		PrivCreateWindowSizeDependentResources();
	}

}

