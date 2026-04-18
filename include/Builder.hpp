#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <type_traits>

#include "Buffer.hpp"

class Builder
{
public:
	template<typename T>
	std::size_t Add(T value)
	{
		static_assert(std::is_trivially_copyable_v<std::decay_t<T>>, "only fundamental types allowed");
		Align(alignof(T));
		std::size_t offset = Allocate(sizeof(T));
		memcpy(m_data.data() + offset, &value, sizeof(T));
		return offset;
	}

	std::size_t AddString(std::string_view str)
	{
		std::size_t offset = Add<uint32_t>(str.size());
		m_data.resize(m_data.size() + str.size());
		memcpy(m_data.data() + m_data.size() - str.size(), str.data(), str.size());
		return offset;
	}

	Buffer Finish()
	{
		return Buffer(std::move(m_data));
	}

private:
	std::vector <uint8_t> m_data;

	void Align(std::size_t alignment)
	{
		auto new_placement = (m_data.size() + alignment - 1) & ~(alignment - 1);
		m_data.resize(new_placement);
	}

	std::size_t Allocate(std::size_t bytes)
	{
		std::size_t offset = m_data.size();
		m_data.resize(m_data.size() + bytes);
		return offset;
	}
};