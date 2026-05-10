#pragma once
#include <span>
#include <string_view>
#include <cstdint>

class StringArrayView
{
public:
	StringArrayView(std::span<const uint8_t> data, std::span<const uint32_t> str_offsets) : m_data(data), m_str_offsets(str_offsets) {};

	std::string_view operator[](std::size_t i) const
	{
		uint32_t a_offset = m_str_offsets[i];
		uint32_t len = *reinterpret_cast<const uint32_t*>(m_data.data() + a_offset);
		return { reinterpret_cast<const char*>(m_data.data() + a_offset + sizeof(uint32_t)), len };
	}

	std::size_t size() const
	{
		return m_str_offsets.size();
	}

	bool empty() const
	{
		return m_str_offsets.empty();
	}

	struct Iterator
	{
		const StringArrayView* view;
		std::size_t index;

		std::string_view operator*()
		{
			return (*view)[index];
		}

		Iterator& operator++()
		{
			++index;
			return *this;
		}

		bool operator!=(const Iterator& other) const
		{
			return index != other.index;
		}
	};

	Iterator begin() const 
	{ 
		return { this, 0 }; 
	}

	Iterator end() const 
	{ 
		return { this, m_str_offsets.size() };
	}
	
private:
	std::span<const uint8_t> m_data;
	std::span<const uint32_t> m_str_offsets;
};