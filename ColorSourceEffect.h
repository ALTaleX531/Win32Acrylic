#pragma once
#include "CompositionEffect.h"

class ColorSourceEffect : public CompositionEffect
{
public:
	ColorSourceEffect() : CompositionEffect(CLSID_D2D1Flood)
	{
		SetColor();
	}
	virtual ~ColorSourceEffect() = default;

	void SetColor(const D2D1_COLOR_F& color = {0.f, 0.f, 0.f, 1.f})
	{
		float value[] = {color.r, color.g, color.b, color.a};
		SetProperty(D2D1_FLOOD_PROP_COLOR, Property(value));
	}
};