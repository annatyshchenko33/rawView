#pragma once
#include <cstdint>
#include <span>
#include <string_view>
#include <stdexcept>
#include <vector>

#include "Buffer.hpp"
#include <StringArrayView.hpp>

class FieldProxy;

class View
{
public:
	View(const Buffer& buffer) : m_data(buffer.raw_bytes()) 
	{
		m_root_offset = Read<uint32_t>(0);
	};

	FieldProxy operator[](std::string_view name);

	template<typename T>
	const T& Read(std::size_t offset)
	{
		if (!(offset + sizeof(T) <= m_data.size()))
		{
			throw std::out_of_range("Out of range(Read)");
		}

		return *reinterpret_cast<const T*>(m_data.data() + offset);
	}

	std::string_view ReadString(std::size_t offset)
	{
		uint32_t str_len = Read<uint32_t>(offset);
		std::size_t char_offset = offset + sizeof(uint32_t);

		if (!((char_offset + str_len) <= m_data.size()))
		{
			throw std::out_of_range("Out of range(ReadString)");
		}

		return std::string_view(reinterpret_cast<const char*> (m_data.data() + char_offset), str_len);
	}

	template<typename T>
	std::span<const T> ReadArray(std::size_t offset)
	{
		uint32_t arr_len = Read<uint32_t>(offset);
		std::size_t arr_offset = offset + sizeof(uint32_t);

		if (!((arr_offset + arr_len * sizeof(T)) <= m_data.size()))
		{
			throw std::out_of_range("Out of range(ReadArr)");
		}

		return std::span(reinterpret_cast<const T*>(m_data.data() + arr_offset), arr_len);
	}

	//index search
	template <typename T>
	const T& ReadTable(std::size_t field_index)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, field_index);
		return Read<T>(field_offset);
	}

	std::string_view ReadTableString(std::size_t field_index)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, field_index);
		return ReadString(field_offset);
	}

	template<typename T>
	std::span<const T> ReadTableArr(std::size_t field_index)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, field_index);
		return ReadArray<T>(field_offset);
	}

	//field name search
	template <typename T>
	const T& ReadTable(std::string_view name)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, name);
		return Read<T>(field_offset);
	}

	std::string_view ReadTableString(std::string_view name)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, name);
		return ReadString(field_offset);
	}

	template<typename T>
	std::span<const T> ReadTableArr(std::string_view name)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, name);
		return ReadArray<T>(field_offset);
	}

	View SubView(std::size_t table_offset)
	{
		return View(m_data, table_offset);
	}

	std::span<const uint8_t> raw_bytes() const noexcept
	{
		return m_data;
	}

	StringArrayView ReadStringArray(std::size_t offset)
	{
		uint32_t count = Read<uint32_t>(offset);

		if (count == 0)
		{
			return StringArrayView(m_data, {});
		}

		std::size_t offsets_start = offset + sizeof(uint32_t);
		if (offsets_start + count * sizeof(uint32_t) > m_data.size())
		{
			throw std::out_of_range("Out of range(ReadStringView)");
		}

		auto offsets = std::span<const uint32_t>(reinterpret_cast<const uint32_t*>(m_data.data() + offsets_start), count);
		return StringArrayView(m_data, offsets);
	}

	StringArrayView ReadTableStringArray(std::string_view name)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, name);
		return ReadStringArray(field_offset);
	}

	StringArrayView ReadTableStringArray(std::size_t offset)
	{
		std::size_t field_offset = GetFieldOffset(m_root_offset, offset);
		return ReadStringArray(field_offset);
	}

	uint32_t field_count()
	{
		return Read<uint32_t>(m_root_offset);
	}

	bool has(std::string_view name)
	{
		uint32_t field_count = Read<uint32_t>(m_root_offset);
		std::size_t first_pair = m_root_offset + sizeof(uint32_t);

		for (std::size_t i = 0; i < field_count; ++i)
		{
			uint32_t name_offset = Read<uint32_t>(first_pair);
			if (ReadString(name_offset) == name)
			{
				return true;
			}
			first_pair += 8;
		}
		return false;
	}

	std::vector<std::string_view> keys()
	{
		uint32_t field_count = Read<uint32_t>(m_root_offset);
		std::vector<std::string_view> result;
		result.reserve(field_count);
		std::size_t first_pair = m_root_offset + sizeof(uint32_t);

		for (std::size_t i = 0; i < field_count; ++i)
		{
			uint32_t name_offset = Read<uint32_t>(first_pair);
			result.push_back(ReadString(name_offset));
			first_pair += 8;
		}
		return result;
	}

private:
	View(std::span<const uint8_t> data, std::size_t table_offset)
		: m_data(data), m_root_offset(table_offset) {}

	std::span <const uint8_t> m_data;
	std::size_t m_root_offset;

	std::size_t GetFieldOffset(std::size_t table_offset, std::size_t field_index)
	{
		uint32_t field_count = Read<uint32_t>(table_offset);
		if (field_index >= field_count)
		{
			throw std::out_of_range("field index out of range");
		}

		std::size_t table_offset_start = table_offset + sizeof(uint32_t);
		std::size_t field_offset = Read<uint32_t>(table_offset_start + field_index * sizeof(uint32_t) * 2 + sizeof(uint32_t));

		return field_offset;
	}

	std::size_t GetFieldOffset(std::size_t table_offset, std::string_view name)
	{
		uint32_t field_count = Read<uint32_t>(table_offset);
		std::size_t table_offset_start = table_offset + sizeof(uint32_t);

		for (std::size_t i = 0; i < field_count; ++i)
		{
			uint32_t name_offset = Read<uint32_t>(table_offset_start);
			uint32_t value_offset = Read<uint32_t>(table_offset_start + sizeof(uint32_t));

			if (ReadString(name_offset) == name)
			{
				return value_offset;

			}
			table_offset_start += sizeof(uint32_t) * 2;
		}
		throw std::out_of_range("cannot find this field by name");
	}
};

#include "FieldProxy.hpp"

inline FieldProxy View::operator[](std::string_view name)
{
	std::size_t offset = GetFieldOffset(m_root_offset, name);
	return FieldProxy(*this, offset);
}