#pragma once

#include <unordered_map>
#include <cstdint>
#include <string_view>
#include <stdexcept>

#include "View.hpp"
#include "Buffer.hpp"

class HashedView
{
public:
	HashedView(const Buffer& buf) : m_view(buf)
	{
		for (auto [name, offset] : m_view.field_entries())
			m_index.insert({ name, offset });
	}

	//by name (O(1) lookup)

	template<typename T>
		requires SupportedScalar<T>
	const T& ReadTable(std::string_view name)
	{
		return m_view.Read<T>(find(name));
	}

	std::string_view ReadTableString(std::string_view name)
	{
		return m_view.ReadString(find(name));
	}

	template<typename T>
		requires SupportedScalar<T>
	std::span<const T> ReadTableArr(std::string_view name)
	{
		return m_view.ReadArray<T>(find(name));
	}

	template<RawStruct T>
	const T& ReadTableStruct(std::string_view name)
	{
		return m_view.Read<T>(find(name));
	}

	template<RawStruct T>
	std::span<const T> ReadTableStructArray(std::string_view name)
	{
		return m_view.ReadStructArray<T>(find(name));
	}

	StringArrayView ReadTableStringArray(std::string_view name)
	{
		return m_view.ReadStringArray(find(name));
	}

	FieldProxy operator[](std::string_view name)
	{
		return FieldProxy(m_view, find(name));
	}

	bool has(std::string_view name) const
	{
		return m_index.count(name) > 0;
	}

	//by index

	template<typename T>
		requires SupportedScalar<T>
	const T& ReadTable(std::size_t index)
	{
		return m_view.ReadTable<T>(index);
	}

	std::string_view ReadTableString(std::size_t index)
	{
		return m_view.ReadTableString(index);
	}

	template<typename T>
		requires SupportedScalar<T>
	std::span<const T> ReadTableArr(std::size_t index)
	{
		return m_view.ReadTableArr<T>(index);
	}

	template<RawStruct T>
	const T& ReadTableStruct(std::size_t index)
	{
		return m_view.ReadTableStruct<T>(index);
	}

	template<RawStruct T>
	std::span<const T> ReadTableStructArray(std::size_t index)
	{
		return m_view.ReadTableStructArray<T>(index);
	}

	StringArrayView ReadTableStringArray(std::size_t index)
	{
		return m_view.ReadTableStringArray(index);
	}

	uint32_t field_count()
	{
		return m_view.field_count();
	}

private:
	View m_view;
	std::unordered_map<std::string_view, uint32_t> m_index;

	uint32_t find(std::string_view name) const
	{
		auto it = m_index.find(name);
		if (it == m_index.end())
			throw std::out_of_range("cannot find this field by name");
		return it->second;
	}
};
