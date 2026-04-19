#pragma once
#include <cstdint>
#include <span>
#include <string_view>
#include <stdexcept>

#include "Buffer.hpp"

class View
{
public:
	View(const Buffer& buffer) : m_data(buffer.raw_bytes()) 
	{
		m_root_offset = Read<uint32_t>(0);
	};

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

private:
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