#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <stdexcept>

#include "View.hpp"
#include "Builder.hpp"
#include "StringArrayView.hpp"

class FieldProxy
{
public:
	FieldProxy(View& view, std::size_t offset) : m_view(view), m_offset(offset) {};

	template<typename T>
	const T& as()
	{
		return m_view.Read<T>(m_offset);
	}

	std::string_view asString()
	{
		return m_view.ReadString(m_offset);
	}

	template<typename T>
	std::span<const T> asArray()
	{
		return m_view.ReadArray<T>(m_offset);
	}

	template<RawStruct T>
	const T& asStruct()
	{
		return m_view.Read<T>(m_offset);
	}

	template<RawStruct T>
	std::span<const T> asStructArray()
	{
		return m_view.ReadStructArray<T>(m_offset);
	}

	View asTable()
	{
		return m_view.SubView(m_offset);
	}

	StringArrayView asStringArray()
	{
		return m_view.ReadStringArray(m_offset);
	}

private:
	View& m_view;
	std::size_t m_offset;
};

