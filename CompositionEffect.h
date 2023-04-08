#pragma once
#include "winrt.h"
#include "crt.h"

class CompositionEffectSource
{
	ComPtr<ICompositionEffectSourceParameter> m_effectSourceParamter{nullptr};
public:
	CompositionEffectSource(const HSTRING& name)
	{
		ComPtr<ICompositionEffectSourceParameterFactory> effectSourceParameterFactory{nullptr};
		ThrowIfFailed(
			GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Composition_CompositionEffectSourceParameter).Get(), &effectSourceParameterFactory)
		);
		ThrowIfFailed(
			effectSourceParameterFactory->Create(name, &m_effectSourceParamter)
		);
	}

	operator ICompositionEffectSourceParameter*()
	{
		return m_effectSourceParamter.Get();
	}
	operator IGraphicsEffectSource*()
	{
		return TryAs<IGraphicsEffectSource>(m_effectSourceParamter).Get();
	}
};

class CompositionEffect : public RuntimeClass<RuntimeClassFlags<WinRtClassicComMix>, IGraphicsEffect, IGraphicsEffectSource, IGraphicsEffectD2D1Interop>
{
public:
	CompositionEffect(REFCLSID effectId) : m_effectId(effectId)
	{
		ThrowIfFailed(
			GetActivationFactory(
				HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(),
				&m_propertyValueFactory
			)
		);
	}
	virtual ~CompositionEffect() = default;

	// IGraphicsEffect
	HRESULT STDMETHODCALLTYPE get_Name(HSTRING* name) override
	{
		return WindowsDuplicateString(m_name, name);
	}
	HRESULT STDMETHODCALLTYPE put_Name(HSTRING name) override
	{
		return m_name.Set(name);
	}
	//

	// IGraphicsEffectD2D1Interop
	HRESULT STDMETHODCALLTYPE GetEffectId(GUID* id) override
	{
		if (id)
		{
			*id = m_effectId;
			return S_OK;
		}

		return E_POINTER;
	}
	// set property by using name, this is not neccessary for C++ programmers
	HRESULT STDMETHODCALLTYPE GetNamedPropertyMapping(LPCWSTR name, UINT* index, GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping) override
	{
		return E_NOTIMPL;
	}
	HRESULT STDMETHODCALLTYPE GetPropertyCount(UINT* count) override
	{
		if (count)
		{
			*count = (UINT)m_properties.size();
			return S_OK;
		}

		return E_POINTER;
	}
	HRESULT STDMETHODCALLTYPE GetProperty(UINT index, IPropertyValue** value) override
	{
		if (!value)
		{
			return E_POINTER;
		}

		if (index < 0 or (index != 0 and m_properties.find(index - 1) == m_properties.end()))
		{
			return E_INVALIDARG;
		}

		return m_properties[index].CopyTo(value);
	}
	HRESULT STDMETHODCALLTYPE GetSource(UINT index, IGraphicsEffectSource** source) override
	{
		if (!source)
		{
			return E_POINTER;
		}

		if (index >= m_effectSources.size() or index < 0)
		{
			return E_INVALIDARG;
		}

		return m_effectSources[index].CopyTo(source);
	}
	HRESULT STDMETHODCALLTYPE GetSourceCount(UINT* count) override
	{
		if (count)
		{
			*count = (UINT)m_effectSources.size();
			return S_OK;
		}

		return E_POINTER;
	}
	//

	void SetInput(UINT index, IGraphicsEffectSource* source)
	{
		if (index < 0 or (index != 0 and m_effectSources.find(index - 1) == m_effectSources.end()))
		{
			ThrowIfFailed(E_INVALIDARG);
		}

		m_effectSources[index] = {ComPtr<IGraphicsEffectSource>(source)};
	}
	void SetInput(IGraphicsEffectSource* source)
	{
		SetInput(0, source);
	}

	void SetInput(const vector<IGraphicsEffectSource*>& effectSourceList)
	{
		for (UINT i = 0; i < effectSourceList.size(); i++)
		{
			SetInput(i, effectSourceList[i]);
		}
	}
	void SetInput(const vector<ComPtr<IGraphicsEffectSource>>& effectSourceList)
	{
		for (UINT i = 0; i < effectSourceList.size(); i++)
		{
			SetInput(i, effectSourceList[i].Get());
		}
	}
protected:
	template <typename T>
	ComPtr<IPropertyValue> Property(T value)
	{
		ThrowIfFailed(E_INVALIDARG);
		return nullptr;
	}
	template <typename T, size_t size>
	ComPtr<IPropertyValue> Property(T(&array)[size])
	{
		ThrowIfFailed(E_INVALIDARG);
		return nullptr;
	}
	template <>
	ComPtr<IPropertyValue> Property(float value)
	{
		ComPtr<IPropertyValue> propertyValue{nullptr};
		ThrowIfFailed(
			m_propertyValueFactory->CreateSingle(value, &propertyValue)
		);
		return propertyValue;
	}
	template <>
	ComPtr<IPropertyValue> Property(UINT32 value)
	{
		ComPtr<IPropertyValue> propertyValue{nullptr};
		ThrowIfFailed(
			m_propertyValueFactory->CreateUInt32(value, &propertyValue)
		);
		return propertyValue;
	}
	template <>
	ComPtr<IPropertyValue> Property(boolean value)
	{
		ComPtr<IPropertyValue> propertyValue{nullptr};
		ThrowIfFailed(
			m_propertyValueFactory->CreateBoolean(value, &propertyValue)
		);
		return propertyValue;
	}
	template <size_t size>
	ComPtr<IPropertyValue> Property(float(&array)[size])
	{
		ComPtr<IPropertyValue> propertyValue{nullptr};
		ThrowIfFailed(
			m_propertyValueFactory->CreateSingleArray(size, array, &propertyValue)
		);
		return propertyValue;
	}

	void SetProperty(UINT index, const ComPtr<IPropertyValue>& value)
	{
		m_properties[index] = value;
	}

	CLSID												m_effectId{};
	HString												m_name{};

	unordered_map<int, ComPtr<IPropertyValue>>			m_properties{};
	ComPtr<IPropertyValueStatics>						m_propertyValueFactory{nullptr};

	unordered_map<int, ComPtr<IGraphicsEffectSource>>	m_effectSources{};
};