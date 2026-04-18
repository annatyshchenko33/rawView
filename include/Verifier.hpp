#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include "Buffer.hpp"

class Verifier
{
public:
	Verifier(const Buffer& buffer) : m_data(buffer.raw_bytes()) {};

	template<typename T>
	bool Verify(std::size_t offset)
	{
		return VerifyRange(offset, sizeof(T));
	}

	bool VerifyString(std::size_t offset)
	{
		if (!Verify<uint32_t>(offset))
		{
			return false;
		}

		const uint32_t str_len = *reinterpret_cast<const uint32_t*>(m_data.data() + offset);

		if (!VerifyRange(offset + sizeof(uint32_t), str_len))
		{
			return false;
		}

		return true;
	}

	bool VerifyBuffer()
	{
		return !m_data.empty();
	}

private:
	std::span<const uint8_t> m_data;

	template<typename T>
	bool VerifyRange(std::size_t offset, std::size_t size)
	{
		if (offset + size <= m_data.size())
		{
			return true;
		}
		return false;
	}
};