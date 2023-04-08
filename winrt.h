#pragma once
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <unknwn.h>
#include <tchar.h>

#include <wrl.h>
#include <wrl\client.h>
#include <wrl\module.h>
#include <wrl\implements.h>
#include <wrl\wrappers\corewrappers.h>

#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <d3d11_4.h>
#include <dcomp.h>
#include <wincodec.h>

#include <roapi.h>
#include <windowsx.h>
#include <DispatcherQueue.h>

#include <windows.foundation.h>
#include <windows.foundation.collections.h>

#include <windows.ui.composition.h>
#include <windows.ui.composition.desktop.h>
#include <windows.ui.composition.interop.h>
#include <windows.ui.composition.effects.h>
#include <windows.ui.composition.interactions.h>

#include <windows.graphics.h>
#include <windows.graphics.effects.h>
#include <windows.graphics.effects.interop.h>

#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Foundation.Collections.h>

#include <winrt\Windows.UI.Composition.Desktop.h>
#include <winrt\Windows.UI.Composition.effects.h>
#include <winrt\Windows.UI.Composition.interactions.h>
#include <winrt\Windows.UI.Composition.h>
#include <winrt\Windows.UI.Composition.h>

#include <winrt\Windows.Graphics.h>
#include <winrt\Windows.Graphics.Effects.h>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dcomp")
#pragma comment(lib, "windowsapp")

namespace
{
	using namespace ABI::Windows::System;

	using namespace ABI::Windows::Foundation;
	using namespace ABI::Windows::Foundation::Numerics;

	using namespace ABI::Windows::UI;
	using namespace ABI::Windows::UI::Composition;
	using namespace ABI::Windows::UI::Composition::Effects;
	using namespace ABI::Windows::UI::Composition::Desktop;

	using namespace ABI::Windows::Graphics;
	using namespace ABI::Windows::Graphics::DirectX;
	using namespace ABI::Windows::Graphics::Effects;

	using namespace Microsoft::WRL;
	using namespace Microsoft::WRL::Wrappers;
	using namespace D2D1;

	static inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw hr;
		}
	}

	template <typename To, typename From>
	static inline auto TryAs(const From* ptr)
	{
		ComPtr<To> instance{nullptr};
		ThrowIfFailed(
			((IUnknown*)ptr)->QueryInterface(IID_PPV_ARGS(&instance))
		);
		return instance.Get();
	}

	template <typename To, typename From>
	static inline auto TryAs(const ComPtr<From>& ptr)
	{
		ComPtr<To> instance{nullptr};
		ThrowIfFailed(
			ptr->QueryInterface(IID_PPV_ARGS(&instance))
		);
		return instance;
	}
}