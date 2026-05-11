#pragma once

#include <cstdint>
#include <span>

#include "Buffer.hpp"

class Verifier
{
public:
	Verifier(const Buffer& buffer) : m_data(buffer.raw_bytes()) {};
	
	template<typename T>
	bool Verify(std::size_t offset) const
	{
		return InBounds(offset, sizeof(T));
	}

	bool VerifyString(std::size_t offset) const
	{
		if (!InBounds(offset, sizeof(uint32_t)))
		{
			return false;
		}

		const uint32_t str_len = *reinterpret_cast<const uint32_t*>(m_data.data() + offset);

		if (!InBounds(offset + sizeof(uint32_t), str_len))
		{
			return false;
		}

		return true;
	}

	bool VerifyTable(std::size_t table_offset) const 
	{
		if (!InBounds(table_offset, sizeof(uint32_t)))
		{
			return false;
		}

		uint32_t field_count = *(uint32_t*)(m_data.data() + table_offset);

		if (field_count > m_data.size() / 8)
		{
			return false;
		}

		std::size_t desc_start = table_offset + sizeof(uint32_t);
		if (!InBounds(desc_start, field_count * 8))
		{
			return false;
		}

		for (std::size_t i = 0; i < field_count; ++i)
		{
			uint32_t name_offset = *(uint32_t*)(m_data.data() + desc_start + i * 8 + 0);
			if (!VerifyString(name_offset)) 
			{
				return false;
			}
		}

		return true;
	}

	bool VerifyBuffer() const 
	{
		if (!InBounds(0, sizeof(uint32_t)))
		{
			return false;
		}

		const uint32_t root_offset = *reinterpret_cast<const uint32_t*>(m_data.data());

		return VerifyTable(root_offset);
	}

private:
	std::span<const uint8_t> m_data;

	bool InBounds(std::size_t offset, std::size_t size) const
	{
		if (size <= m_data.size() && offset <= m_data.size() -size)
		{
			return true;
		}
		return false;
	}
};