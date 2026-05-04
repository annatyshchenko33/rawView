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

	template<typename T>
	std::size_t AddArray(std::span<const T> arr)
	{
		std::size_t offset = Add<uint32_t>(arr.size());
		for (auto& elem : arr)
		{
			Add<T>(elem);
		}
		return offset;
	}
	
	//initializer_list
	template<typename T>
	std::size_t AddArray(std::initializer_list<T> arr)
	{
		return AddArray<T>(std::span<const T>(arr.begin(), arr.size()));
	}

	Buffer Finish()
	{
		return Buffer(std::move(m_data));
	}

	template<typename T>
	void WriteAt(std::size_t offset, T value)
	{
		memcpy(m_data.data() + offset, &value, sizeof(T));
	}

	std::size_t Embed(const Builder& other, std::size_t skip_bytes)
	{
		auto aligned = (m_data.size() + alignof(uint32_t) - 1) & ~(alignof(uint32_t) - 1);
		m_data.resize(aligned);

		std::size_t start_offset = m_data.size();
		auto inner_part = std::span<const uint8_t>(other.m_data).subspan(skip_bytes);
		m_data.insert(m_data.end(), inner_part.begin(), inner_part.end());
		return start_offset;
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