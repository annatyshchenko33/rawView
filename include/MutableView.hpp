#pragma once

#include <span>
#include <string_view>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "Buffer.hpp"
#include "View.hpp" 

class MutableView
{
public:
	MutableView(Buffer& buffer) 
	{
		m_data = buffer.mutable_bytes();
		if (m_data.size() < sizeof(uint32_t))
		{
			throw std::invalid_argument("invalid argument");
		}
		m_root_offset = Read<uint32_t>(0);
	}

	template<typename T>
	void Set(std::string_view name, T value)
	{
		static_assert(std::is_trivially_copyable_v<std::decay_t<T>>, "T must be trivially copyable");
		std::size_t offset = GetFieldOffset(name);
		Read<T>(offset) = value;
	}

	template<typename T>
	void Set(std::size_t index, T value)
	{
		static_assert(std::is_trivially_copyable_v<std::decay_t<T>>, "T must be trivially copyable");
		std::size_t offset = GetFieldOffset(index);
		Read<T>(offset) = value;
	}

	void SetString(std::string_view name, std::string_view str)
	{
		std::size_t offset = GetFieldOffset(name);
		uint32_t existing_len = Read<uint32_t>(offset);

		if (!(str.size() == existing_len))
		{
			throw std::invalid_argument("SetString: length must match");
		}
		memcpy(m_data.data() + offset + sizeof(uint32_t), str.data(), str.size());
	}

	void SetString(std::size_t index, std::string_view str)
	{
		std::size_t offset = GetFieldOffset(index);
		uint32_t existing_len = Read<uint32_t>(offset);

		if (!(str.size() == existing_len))
		{
			throw std::invalid_argument("SetString: length must match");
		}
		memcpy(m_data.data() + offset + sizeof(uint32_t), str.data(), str.size());
	}

	template<typename T>
	T Get(std::string_view name)
	{
		static_assert(std::is_trivially_copyable_v<std::decay_t<T>>, "T must be trivially copyable");
		std::size_t offset = GetFieldOffset(name);
		return Read<T>(offset);
	}

	template<typename T>
	T Get(std::size_t index)
	{
		static_assert(std::is_trivially_copyable_v<std::decay_t<T>>, "T must be trivially copyable");
		std::size_t offset = GetFieldOffset(index);
		return Read<T>(offset);
	}

	std::string_view GetString(std::string_view name)
	{
		std::size_t offset = GetFieldOffset(name);
		return ReadString(offset);
	}

	std::string_view GetString(std::size_t index)
	{
		std::size_t offset = GetFieldOffset(index);
		return ReadString(offset);
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

	View AsView() const
	{
		Buffer tmp = Buffer::borrow(m_data.data(), m_data.size());
		return View(tmp);
	}

	MutableView GetSubMutable(std::string_view name)
	{
		std::size_t offset = GetFieldOffset(name);
		return  MutableView(m_data, offset);
	}

	MutableView GetSubMutable(std::size_t index)
	{
		std::size_t offset = GetFieldOffset(index);
		return  MutableView(m_data, offset);
	}

private:
	MutableView(std::span<uint8_t> data, std::size_t root_offset)
	{
		m_data = data;
		m_root_offset = root_offset;
	}

	std::span<uint8_t> m_data;
	std::size_t m_root_offset;

	template<typename T>
	T& Read(std::size_t offset)
	{
		if (!(offset + sizeof(T) <= m_data.size()))
		{
			throw std::out_of_range("Out of range(Read)");
		}

		return *reinterpret_cast<T*>(m_data.data() + offset);
	}

	std::string_view ReadString(std::size_t offset)
	{
		uint32_t str_len = Read<uint32_t>(offset);
		std::size_t char_offset = offset + sizeof(uint32_t);

		if (!((char_offset + str_len) <= m_data.size()))
		{
			throw std::out_of_range("Out of range(ReadString)");
		}

		return std::string_view(reinterpret_cast<char*> (m_data.data() + char_offset), str_len);
	}

	std::size_t GetFieldOffset(std::string_view name)
	{
		uint32_t field_count = Read<uint32_t>(m_root_offset);
		std::size_t first_pair = m_root_offset + sizeof(uint32_t);

		for (std::size_t i = 0; i < field_count; ++i)
		{
			uint32_t name_offset = Read<uint32_t>(first_pair);
			uint32_t value_offset = Read<uint32_t>(first_pair + sizeof(uint32_t));
			if (ReadString(name_offset) == name)
			{
				return value_offset;
			}
			first_pair += sizeof(uint32_t) * 2;
		}
		throw std::runtime_error("field not found: " + std::string(name));
	}

	std::size_t GetFieldOffset(std::size_t field_index)
	{
		uint32_t field_count = Read<uint32_t>(m_root_offset);
		if (field_index >= field_count)
		{
			throw std::out_of_range("field index out of range");
		}

		std::size_t table_offset_start = m_root_offset + sizeof(uint32_t);
		std::size_t field_offset = Read<uint32_t>(table_offset_start + field_index * sizeof(uint32_t) * 2 + sizeof(uint32_t));

		return field_offset;
	}
}; 