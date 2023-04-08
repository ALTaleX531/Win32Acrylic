#pragma once
#include "CompositionEffect.h"

class SaturationEffect : public CompositionEffect
{
public:
	SaturationEffect() : CompositionEffect(CLSID_D2D1Saturation)
	{
		SetSaturation();
	}
	virtual ~SaturationEffect() = default;

	void SetSaturation(float saturation = 0.5f)
	{
		SetProperty(D2D1_SATURATION_PROP_SATURATION, Property(saturation));
	}
};