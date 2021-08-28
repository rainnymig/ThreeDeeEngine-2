//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT _WIN32_WINNT_WINBLUE
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

#include <wrl/client.h>

#include <d3d11_1.h>
#include <d3dcompiler.h>

#if defined(NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#include <dxgi1_5.h>
#endif

#include <DirectXMath.h>
#include <DirectXColors.h>

#include "Mouse.h"
#include "Keyboard.h"
#include "DirectXHelpers.h"
#include "CommonStates.h"
#include "SimpleMath.h"

#include "common/stb_image.h"

#include <algorithm>
#include <functional>
#include <cmath>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <list>
#include <deque>
#include <unordered_map>

#include <cstdio>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

// Link library dependencies
#pragma comment(lib, "d3dcompiler.lib")

#include "common/ConstructorTagHelper.h"

#define RETURN_IF_FAILED(hr) if(FAILED(hr)){return hr;}

#define SAFE_RELEASE(aComPtr) {if(aComPtr){aComPtr.Reset();}}

namespace tde
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) noexcept : result(hr) {}

        const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}
