#pragma once

#include <string_view>
#include "Builder.hpp"

class TableBuilder
{
public:
	TableBuilder(Builder& builder) :m_builder(builder) {};

	template <typename T>
	TableBuilder& Add(T value)
	{
		std::size_t offset = m_builder.Add(value);
		m_offsets.push_back(offset);
		return *this;
	}

	TableBuilder& AddString(std::string_view str)
	{
		std::size_t offset = m_builder.AddString(str);
		m_offsets.push_back(offset);
		return *this;
	}

	std::pair<Buffer, std::size_t> Finish()
	{
		std::size_t table_offset = m_builder.Add<uint32_t>(m_offsets.size());

		for (int i = 0; i < m_offsets.size(); ++i)
		{
			m_builder.Add<uint32_t>(m_offsets[i]);
		}
		return { m_builder.Finish(), table_offset };
	}

private:
	Builder& m_builder;
	std::vector<std::size_t> m_offsets;
};