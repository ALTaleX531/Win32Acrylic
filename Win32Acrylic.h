#pragma once
#include "winrt.h"
#include "Effects.h"

class Win32Acrylic
{
	ComPtr<IDispatcherQueueController>	m_controller{nullptr};
	ComPtr<IDesktopWindowTarget>		m_target{nullptr};
	ComPtr<ICompositor>					m_compositor{nullptr};

	ComPtr<ICompositionGraphicsDevice>	m_compositionDevice{nullptr};
	ComPtr<ID3D11Device>				m_d3dDevice{nullptr};
	ComPtr<IDXGIDevice>					m_dxgiDevice{nullptr};
	ComPtr<ID2D1Device>					m_d2dDevice{nullptr};

	ComPtr<IWICImagingFactory2>			m_wicFactory{nullptr};
public:
	Win32Acrylic(HWND hWnd)
	{
		// create dispatcher queue
		DispatcherQueueOptions options
		{
			sizeof(DispatcherQueueOptions),
			DQTYPE_THREAD_CURRENT,
			DQTAT_COM_STA
		};
		ThrowIfFailed(CreateDispatcherQueueController(options, &m_controller));

		// create a compositor
		ThrowIfFailed(
			ActivateInstance(
				HStringReference(RuntimeClass_Windows_UI_Composition_Compositor).Get(),
				&m_compositor
			)
		);

		PrepareDevice();

		// create desktop window tarGet
		ThrowIfFailed(
			TryAs<ICompositorDesktopInterop>(m_compositor)->CreateDesktopWindowTarget(
				hWnd, TRUE, &m_target
			)
		);

		// create a sprite visual as root visual which host acrylic content
		ComPtr<ISpriteVisual> rootVisual{nullptr};
		ThrowIfFailed(
			m_compositor->CreateSpriteVisual(&rootVisual)
		);
		ThrowIfFailed(
			TryAs<IVisual2>(rootVisual)->put_RelativeSizeAdjustment({1.f, 1.f})
		);
		ThrowIfFailed(
			rootVisual->put_Brush(CreateAcrylicBrush().Get())
		);

		// now set it as root visual
		ThrowIfFailed(
			TryAs<ICompositionTarget>(m_target)->put_Root(TryAs<IVisual>(rootVisual).Get())
		);
	}
	~Win32Acrylic() = default;

	// create composition graphics device, d3d device and d2d device
	void PrepareDevice()
	{

		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		ThrowIfFailed(
			D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&m_d3dDevice,
				nullptr,
				nullptr
			)
		);

		ThrowIfFailed(
			m_d3dDevice.As(&m_dxgiDevice)
		);

		ThrowIfFailed(
			D2D1CreateDevice(
				m_dxgiDevice.Get(),
				D2D1_CREATION_PROPERTIES
				{
					D2D1_THREADING_MODE_SINGLE_THREADED,
					D2D1_DEBUG_LEVEL_NONE,
					D2D1_DEVICE_CONTEXT_OPTIONS_NONE
				},
				&m_d2dDevice
			)
		);

		auto interopCompositor{TryAs<ICompositorInterop>(m_compositor)};
		ThrowIfFailed(interopCompositor->CreateGraphicsDevice(m_d2dDevice.Get(), &m_compositionDevice));
		ThrowIfFailed(
			CoCreateInstance(
				CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicFactory)
			)
		);
	}

	ComPtr<ICompositionSurfaceBrush> CreateNoiceBrush()
	{
		// create drawing surface
		ComPtr<ICompositionDrawingSurface> compositionSurface{nullptr};
		ThrowIfFailed(
			m_compositionDevice->CreateDrawingSurface(
				{256, 256},
				DirectXPixelFormat::DirectXPixelFormat_R16G16B16A16Float,
				DirectXAlphaMode::DirectXAlphaMode_Premultiplied,
				&compositionSurface
			)
		);

		// create surface brush
		ComPtr<ICompositionSurfaceBrush> surfaceBrush{nullptr};
		ThrowIfFailed(
			m_compositor->CreateSurfaceBrushWithSurface(TryAs<ICompositionSurface>(compositionSurface).Get(), &surfaceBrush)
		);

		// get surface interop interface
		auto m_surfaceInterop{TryAs<ICompositionDrawingSurfaceInterop>(compositionSurface)};

		// load shared noice texture from system components
		HINSTANCE hModule = LoadLibraryEx(_T("Windows.UI.Xaml.Controls.dll"), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
		ThrowIfFailed(hModule ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

		HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(2000), RT_RCDATA);
		ThrowIfFailed(hResource ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

		HGLOBAL hGlobal = LoadResource(hModule, hResource);
		ThrowIfFailed(hGlobal ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

		DWORD dwResourceSize = SizeofResource(hModule, hResource);
		ThrowIfFailed(dwResourceSize ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

		BYTE* pbResource = (BYTE*)LockResource(hGlobal);
		ThrowIfFailed(pbResource ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

		ComPtr<IStream> stream{SHCreateMemStream(pbResource, dwResourceSize)};
		ThrowIfFailed(stream ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

		UnlockResource(hGlobal);
		FreeResource(hGlobal);
		FreeLibrary(hModule);

		// create direct2d bitmap from wic
		ComPtr<IWICBitmapDecoder> wicDecorder{nullptr};
		ThrowIfFailed(m_wicFactory->CreateDecoderFromStream(stream.Get(), &GUID_VendorMicrosoft, WICDecodeMetadataCacheOnDemand, &wicDecorder));

		ComPtr<IWICBitmapFrameDecode> wicFrame{nullptr};
		ThrowIfFailed(wicDecorder->GetFrame(0, &wicFrame));

		ComPtr<IWICFormatConverter> wicConverter;
		ThrowIfFailed(m_wicFactory->CreateFormatConverter(&wicConverter));

		ComPtr<IWICPalette> wicPalette{nullptr};
		ThrowIfFailed(
			wicConverter->Initialize(
				wicFrame.Get(),
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				wicPalette.Get(),
				0, WICBitmapPaletteTypeCustom
			)
		);

		ComPtr<IWICBitmap> wicBitmap{nullptr};
		ThrowIfFailed(m_wicFactory->CreateBitmapFromSource(wicConverter.Get(), WICBitmapCreateCacheOption::WICBitmapNoCache, &wicBitmap));

		// render texture to visual surface
		ComPtr<ID2D1Bitmap1> d2dBitmap{nullptr};
		ComPtr<ID2D1DeviceContext> d2dContext{nullptr};

		POINT offset = {0, 0};
		ThrowIfFailed(
			m_surfaceInterop->BeginDraw(nullptr, IID_PPV_ARGS(&d2dContext), &offset)
		);
		d2dContext->Clear();
		d2dContext->CreateBitmapFromWicBitmap(
			wicBitmap.Get(),
			BitmapProperties1(
				D2D1_BITMAP_OPTIONS_NONE,
				PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
			),
			&d2dBitmap
		);
		d2dContext->DrawBitmap(d2dBitmap.Get());
		ThrowIfFailed(
			m_surfaceInterop->EndDraw()
		);

		ThrowIfFailed(
			surfaceBrush->put_HorizontalAlignmentRatio(0.f)
		);
		ThrowIfFailed(
			surfaceBrush->put_VerticalAlignmentRatio(0.f)
		);

		return surfaceBrush;
	}

	ComPtr<ICompositionEffectBrush> CreateEffectBrush(IGraphicsEffect* graphicsEffect)
	{
		ComPtr<ICompositionEffectBrush> effectBrush{nullptr};
		ComPtr<ICompositionEffectFactory> effectFactory{nullptr};
		ThrowIfFailed(
			m_compositor->CreateEffectFactory(graphicsEffect, &effectFactory)
		);
		ThrowIfFailed(
			effectFactory->CreateBrush(&effectBrush)
		);
		return effectBrush;
	}

	// the following content was copied from microsoft-ui-xaml-main\microsoft-ui-xaml-main\dev\Materials\Acrylic\AcrylicBrush.cpp
	// ******************************About the Luminosity-based Acrylic Recipe *****************************
	//
	// In 19H1, the Acryic recipe was altered to improve integration of Acrylic surfaces with Shadows using
	// a Luminosity effect. Without this a ThemeShadow cast by an Acrylic surface was visible through it
	// as a dark blur, resulting in a muddied appearence that did not match the Acrylic exppectations.
	// A Luminosity effect is now used to reduce contrast in the Acrylic source and minimize the
	// shadow's contribution to Acrylic output. See comment on Luminosity in recipe description below for details.
	//
	// Since ThemeShadow is only present in 19H1+ OS, the new recipe is only needed when when tarGeting this.
	// In addition, RS2 did not have the needed Luminosity blend mode, so the legacy acrylic path needed to be
	// maintained. For consistency, keep the legacy recipe afor all MUX downlevel configs (i.e. tarGeting RS5 and lower).
	//
	// *************************** Shadow-friendly Luminosity-based Recipe (19H1+) ***************************
	//
	//      <CompositeEffect>           <!-- Provides noise for acrylic -->
	//          <OpacityEffect>     <!-- Noise texture with wrap and alpha -->
	//              <BorderEffect>
	//                  <Noise texture in a brush />
	//              </BorderEffect>
	//          </OpacityEffect>
	//          <BlendEffect (Color)>       <!-- Tint -->
	//              <ColorSourceEffect />   <!-- Tint color -->
	//              <BlendEffect (Luminosity)>
	//                  <ColorSourceEffect />   <!-- Luminosity color -->
	//                  <Blur>
	//                      <Backdrop in a brush/>
	//                  </Blur>
	//              </BlendEffect (Luminosity)>
	//          </BlendEffect (Color)>
	//      </CompositeEffect>
	// ********************************** Legacy Recipe (MUX / RS5 and Lower ) **********************************
	//
	//      <BlendEffect>           <!-- Provides noise for acrylic -->
	//          <OpacityEffect>     <!-- Noise texture with wrap and alpha -->
	//              <BorderEffect>
	//                  <Noise texture in a brush />
	//              </BorderEffect>
	//          </OpacityEffect>
	//          <CompositeStepEffect>       <!-- Tint -->
	//              <ColorSourceEffect />   <!-- Tint color -->
	//              <BlendEffect>               <!-- Exclusion -->
	//                  <ColorSourceEffect />   <!-- Exclusion color -->
	//                  <Saturation>
	//                      <Blur>
	//                          <Backdrop in a brush/>
	//                      </Blur>
	//                  </Saturation>
	//              </BlendEffect>
	//          </CompositeStepEffect>
	//
	//      </BlendEffect>
	ComPtr<ICompositionBrush> CreateAcrylicBrush(D2D1_COLOR_F tintColor = {0.125f, 0.125f, 0.125f, 0.4f}, D2D1_COLOR_F luminosityColor = {0.125f, 0.125f, 0.125f, 0.8f})
	{
		// border effect
		auto borderEffect{Make<BorderEffect>()};
		borderEffect->SetExtendX(D2D1_BORDER_EDGE_MODE_WRAP);
		borderEffect->SetExtendY(D2D1_BORDER_EDGE_MODE_WRAP);
		borderEffect->SetInput(CompositionEffectSource(HStringReference(L"Noice").Get()));
		
		// opacity effect
		auto opacityEffect{Make<OpacityEffect>()};
		opacityEffect->put_Name(HStringReference(L"NoiceOpacity").Get());
		opacityEffect->SetOpacity(0.02f);
		opacityEffect->SetInput(borderEffect.Get());

		// gaussian blur
		auto blurEffect{Make<GaussianBlurEffect>()};
		blurEffect->put_Name(HStringReference(L"Blur").Get());
		blurEffect->SetBlurAmount(30.f);
		blurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
		blurEffect->SetInput(CompositionEffectSource(HStringReference(L"Backdrop").Get()));

		// tint Color
		auto tintColorEffect{Make<ColorSourceEffect>()};
		tintColorEffect->put_Name(HStringReference(L"TintColor").Get());
		tintColorEffect->SetColor(tintColor);

		// luminosity Color
		auto luminosityColorEffect{Make<ColorSourceEffect>()};
		luminosityColorEffect->put_Name(HStringReference(L"LuminosityColor").Get());
		luminosityColorEffect->SetColor(luminosityColor);

		// luminosity blend
		// NOTE: There is currently a bug where the names of BlendEffectMode::Luminosity and BlendEffectMode::Color are flipped.
		// This should be changed to Luminosity when/if the bug is fixed.
		auto luminosityBlendEffect{Make<BlendEffect>()};
		luminosityBlendEffect->SetBlendMode(D2D1_BLEND_MODE_COLOR);
		luminosityBlendEffect->SetBackground(blurEffect.Get());
		luminosityBlendEffect->SetForeground(luminosityColorEffect.Get());

		// color blend
		// NOTE: There is currently a bug where the names of BlendEffectMode::Luminosity and BlendEffectMode::Color are flipped.
		// This should be changed to Color when/if the bug is fixed.
		auto colorBlendEffect{Make<BlendEffect>()};
		colorBlendEffect->SetBlendMode(D2D1_BLEND_MODE_LUMINOSITY);
		colorBlendEffect->SetBackground(luminosityBlendEffect.Get());
		colorBlendEffect->SetForeground(tintColorEffect.Get());

		// noice blend
		auto noiceBlendEffect{Make<BlendEffect>()};
		noiceBlendEffect->SetBlendMode(D2D1_BLEND_MODE_MULTIPLY);
		noiceBlendEffect->SetBackground(colorBlendEffect.Get());
		noiceBlendEffect->SetForeground(opacityEffect.Get());

		ComPtr<ICompositionEffectBrush> effectBrush{CreateEffectBrush(noiceBlendEffect.Get())};
		ComPtr<ICompositionBackdropBrush> backdropBrush{nullptr};
		ComPtr<ICompositionSurfaceBrush> surfaceBrush{CreateNoiceBrush()};
		ThrowIfFailed(
			TryAs<ICompositor2>(m_compositor)->CreateBackdropBrush(&backdropBrush)
		);
		ThrowIfFailed(
			effectBrush->SetSourceParameter(HStringReference(L"Noice").Get(), TryAs<ICompositionBrush>(surfaceBrush).Get())
		); 
		ThrowIfFailed(
			effectBrush->SetSourceParameter(HStringReference(L"Backdrop").Get(), TryAs<ICompositionBrush>(backdropBrush).Get())
		);

		return TryAs<ICompositionBrush>(effectBrush);
	}

	// under Windows 10 1809
	ComPtr<ICompositionBrush> CreateAcrylicBrush_Legacy(D2D1_COLOR_F tintColor = {0.125f, 0.125f, 0.125f, 0.4f}/*{0.95f, 0.95f, 0.95f, 0.4f}*/)
	{
		// border effect
		auto borderEffect{Make<BorderEffect>()};
		borderEffect->SetExtendX(D2D1_BORDER_EDGE_MODE_WRAP);
		borderEffect->SetExtendY(D2D1_BORDER_EDGE_MODE_WRAP);
		borderEffect->SetInput(CompositionEffectSource(HStringReference(L"Noice").Get()));

		// opacity effect
		auto opacityEffect{Make<OpacityEffect>()};
		opacityEffect->put_Name(HStringReference(L"NoiceOpacity").Get());
		opacityEffect->SetOpacity(0.02f);
		opacityEffect->SetInput(borderEffect.Get());
		
		// gaussian blur
		auto blurEffect{Make<GaussianBlurEffect>()};
		blurEffect->put_Name(HStringReference(L"Blur").Get());
		blurEffect->SetBlurAmount(30.f);
		blurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
		blurEffect->SetInput(CompositionEffectSource(HStringReference(L"Backdrop").Get()));

		// saturation
		auto saturationEffect{Make<SaturationEffect>()};
		saturationEffect->put_Name(HStringReference(L"Saturation").Get());
		saturationEffect->SetSaturation(1.25f);
		saturationEffect->SetInput(blurEffect.Get());

		// tint Color
		auto tintColorEffect{Make<ColorSourceEffect>()};
		tintColorEffect->put_Name(HStringReference(L"TintColor").Get());
		tintColorEffect->SetColor(tintColor);

		// exclusion Color
		auto exclusionColorEffect{Make<ColorSourceEffect>()};
		exclusionColorEffect->put_Name(HStringReference(L"ExclusionColor").Get());
		exclusionColorEffect->SetColor({1.f, 1.f, 1.f, 0.1f});

		// exclusion blend
		auto blendEffectInner{Make<BlendEffect>()};
		blendEffectInner->SetBlendMode(D2D1_BLEND_MODE_EXCLUSION);
		blendEffectInner->SetBackground(saturationEffect.Get());
		blendEffectInner->SetForeground(exclusionColorEffect.Get());

		// composite
		auto compositeStepEffect{Make<CompositeStepEffect>()};
		compositeStepEffect->SetCompositeMode(D2D1_COMPOSITE_MODE_SOURCE_OVER);
		compositeStepEffect->SetDestination(blendEffectInner.Get());
		compositeStepEffect->SetSource(tintColorEffect.Get());

		// noice blend
		auto noiceBlendEffect{Make<BlendEffect>()};
		noiceBlendEffect->SetBlendMode(D2D1_BLEND_MODE_MULTIPLY);
		noiceBlendEffect->SetBackground(compositeStepEffect.Get());
		noiceBlendEffect->SetForeground(opacityEffect.Get());

		ComPtr<ICompositionEffectBrush> effectBrush{CreateEffectBrush(noiceBlendEffect.Get())};
		ComPtr<ICompositionBackdropBrush> backdropBrush{nullptr};
		ComPtr<ICompositionSurfaceBrush> surfaceBrush{CreateNoiceBrush()};
		ThrowIfFailed(
			TryAs<ICompositor2>(m_compositor)->CreateBackdropBrush(&backdropBrush)
		);
		ThrowIfFailed(
			effectBrush->SetSourceParameter(HStringReference(L"Noice").Get(), TryAs<ICompositionBrush>(surfaceBrush).Get())
		);
		ThrowIfFailed(
			effectBrush->SetSourceParameter(HStringReference(L"Backdrop").Get(), TryAs<ICompositionBrush>(backdropBrush).Get())
		);

		return TryAs<ICompositionBrush>(effectBrush);
	}

	ComPtr<ICompositionBackdropBrush> CreateBlurredWallpaperBackdropBrush()
	{
		ComPtr<ICompositionBackdropBrush> backdropBrush{nullptr};
		ThrowIfFailed(
			TryAs<ICompositorWithBlurredWallpaperBackdropBrush>(m_compositor)->TryCreateBlurredWallpaperBackdropBrush(&backdropBrush)
		);
		return backdropBrush;
	}

	// since Windows 11
	ComPtr<ICompositionBrush> CreateMicaBrush(D2D1_COLOR_F tintColor = {0.125f, 0.125f, 0.125f, 0.4f}, D2D1_COLOR_F luminosityColor = {0.125f, 0.125f, 0.125f, 0.6f})
	{
		// border effect
		auto borderEffect{Make<BorderEffect>()};
		borderEffect->SetExtendX(D2D1_BORDER_EDGE_MODE_WRAP);
		borderEffect->SetExtendY(D2D1_BORDER_EDGE_MODE_WRAP);
		borderEffect->SetInput(CompositionEffectSource(HStringReference(L"Noice").Get()));

		// opacity effect
		auto opacityEffect{Make<OpacityEffect>()};
		opacityEffect->put_Name(HStringReference(L"NoiceOpacity").Get());
		opacityEffect->SetOpacity(0.02f);
		opacityEffect->SetInput(borderEffect.Get());

		// tint Color
		auto tintColorEffect{Make<ColorSourceEffect>()};
		tintColorEffect->put_Name(HStringReference(L"TintColor").Get());
		tintColorEffect->SetColor(tintColor);

		// luminosity Color
		auto luminosityColorEffect{Make<ColorSourceEffect>()};
		luminosityColorEffect->put_Name(HStringReference(L"LuminosityColor").Get());
		luminosityColorEffect->SetColor(luminosityColor);

		// luminosity blend
		// NOTE: There is currently a bug where the names of BlendEffectMode::Luminosity and BlendEffectMode::Color are flipped.
		// This should be changed to Luminosity when/if the bug is fixed.
		auto luminosityBlendEffect{Make<BlendEffect>()};
		luminosityBlendEffect->SetBlendMode(D2D1_BLEND_MODE_COLOR);
		luminosityBlendEffect->SetBackground(CompositionEffectSource(HStringReference(L"BlurredWallpaperBackdrop").Get()));
		luminosityBlendEffect->SetForeground(luminosityColorEffect.Get());

		// color blend
		// NOTE: There is currently a bug where the names of BlendEffectMode::Luminosity and BlendEffectMode::Color are flipped.
		// This should be changed to Color when/if the bug is fixed.
		auto colorBlendEffect{Make<BlendEffect>()};
		colorBlendEffect->SetBlendMode(D2D1_BLEND_MODE_LUMINOSITY);
		colorBlendEffect->SetBackground(luminosityBlendEffect.Get());
		colorBlendEffect->SetForeground(tintColorEffect.Get());

		// noice blend
		auto noiceBlendEffect{Make<BlendEffect>()};
		noiceBlendEffect->SetBlendMode(D2D1_BLEND_MODE_MULTIPLY);
		noiceBlendEffect->SetBackground(colorBlendEffect.Get());
		noiceBlendEffect->SetForeground(opacityEffect.Get());

		ComPtr<ICompositionEffectBrush> effectBrush{CreateEffectBrush(noiceBlendEffect.Get())};
		ComPtr<ICompositionBackdropBrush> backdropBrush{nullptr};
		ComPtr<ICompositionSurfaceBrush> surfaceBrush{CreateNoiceBrush()};
		ThrowIfFailed(
			TryAs<ICompositor2>(m_compositor)->CreateBackdropBrush(&backdropBrush)
		);
		ThrowIfFailed(
			effectBrush->SetSourceParameter(HStringReference(L"Noice").Get(), TryAs<ICompositionBrush>(surfaceBrush).Get())
		);
		ThrowIfFailed(
			effectBrush->SetSourceParameter(HStringReference(L"BlurredWallpaperBackdrop").Get(), TryAs<ICompositionBrush>(CreateBlurredWallpaperBackdropBrush()).Get())
		);

		return TryAs<ICompositionBrush>(effectBrush);
	}
};

