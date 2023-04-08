#pragma once
#include "CompositionEffect.h"

class OpacityEffect : public CompositionEffect
{
public:
	OpacityEffect() : CompositionEffect(CLSID_D2D1Opacity)
	{
		SetOpacity();
	}
	virtual ~OpacityEffect() = default;

	void SetOpacity(float opacity = 1.0f)
	{
		SetProperty(D2D1_OPACITY_PROP_OPACITY, Property(opacity));
	}
};