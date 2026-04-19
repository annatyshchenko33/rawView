#pragma once

#include <string_view>
#include "Builder.hpp"

class TableBuilder
{
public:
	TableBuilder(Builder& builder) :m_builder(builder) {};

	template <typename T>
	TableBuilder& Add(std::string_view name, T value)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.Add(value);

		FieldInfo field;
		field.name_offset = name_offset;
		field.value_offset = offset;

		m_fields.push_back(field);
		return *this;
	}

	TableBuilder& AddString(std::string_view name, std::string_view str)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.AddString(str);

		FieldInfo field;
		field.name_offset = name_offset;
		field.value_offset = offset;

		m_fields.push_back(field);
		return *this;
	}

	std::pair<Buffer, std::size_t> Finish()
	{
		uint32_t table_offset = m_builder.Add<uint32_t>(m_fields.size());

		for (int i = 0; i < m_fields.size(); ++i)
		{
			m_builder.Add<uint32_t>(m_fields[i].name_offset);
			m_builder.Add<uint32_t>(m_fields[i].value_offset);
		}
		return { m_builder.Finish(), table_offset };
	}

private:
	Builder& m_builder;

	struct FieldInfo
	{
		std::size_t name_offset;
		std::size_t value_offset;
	};

	std::vector<FieldInfo> m_fields;
};