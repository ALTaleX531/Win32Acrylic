#pragma once
#include "CompositionEffect.h"

class BlendEffect : public CompositionEffect
{
public:
	BlendEffect() : CompositionEffect(CLSID_D2D1Blend)
	{
		SetBlendMode();
	}
	virtual ~BlendEffect() = default;

	void SetBlendMode(D2D1_BLEND_MODE blendMode = D2D1_BLEND_MODE_MULTIPLY)
	{
		SetProperty(D2D1_BLEND_PROP_MODE, Property<UINT32>(blendMode));
	}
	void SetBackground(IGraphicsEffectSource* source)
	{
		SetInput(0, source);
	}
	void SetForeground(IGraphicsEffectSource* source)
	{
		SetInput(1, source);
	}
};