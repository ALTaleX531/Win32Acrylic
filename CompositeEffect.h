#pragma once
#include "CompositionEffect.h"

class CompositeEffect : public CompositionEffect
{
public:
	CompositeEffect() : CompositionEffect(CLSID_D2D1Composite)
	{
		SetCompositeMode();
	}
	virtual ~CompositeEffect() = default;

	void SetCompositeMode(D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER)
	{
		SetProperty(D2D1_COMPOSITE_PROP_MODE, Property<UINT32>(compositeMode));
	}
};

class CompositeStepEffect : public CompositionEffect
{
public:
	CompositeStepEffect() : CompositionEffect(CLSID_D2D1Composite)
	{
		SetCompositeMode();
	}
	virtual ~CompositeStepEffect() = default;

	void SetCompositeMode(D2D1_COMPOSITE_MODE compositeMode = D2D1_COMPOSITE_MODE_SOURCE_OVER)
	{
		SetProperty(D2D1_COMPOSITE_PROP_MODE, Property<UINT32>(compositeMode));
	}
	void SetDestination(IGraphicsEffectSource* source)
	{
		SetInput(0, source);
	}
	void SetSource(IGraphicsEffectSource* source)
	{
		SetInput(1, source);
	}
};