#pragma once
#include "CompositionEffect.h"

class GaussianBlurEffect : public CompositionEffect
{
public:
	GaussianBlurEffect() : CompositionEffect(CLSID_D2D1GaussianBlur)
	{
		SetBlurAmount();
		SetOptimizationMode();
		SetBorderMode();
	}
	virtual ~GaussianBlurEffect() = default;

	void SetBlurAmount(float blurAmount = 3.f)
	{
		SetProperty(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, Property(blurAmount));
	}
	void SetOptimizationMode(D2D1_GAUSSIANBLUR_OPTIMIZATION optimizationMode = D2D1_GAUSSIANBLUR_OPTIMIZATION_BALANCED)
	{
		SetProperty(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, Property<UINT32>(optimizationMode));
	}
	void SetBorderMode(D2D1_BORDER_MODE borderMode = D2D1_BORDER_MODE_SOFT)
	{
		SetProperty(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, Property<UINT32>(borderMode));
	}
};