#pragma once

#include <string_view>
#include "Builder.hpp"

template <typename TBuilder = Builder>
class TableBuilder
{
public:
	TableBuilder()
	{
		m_root_offset = m_builder.template Add<uint32_t>(0);
	};

	TableBuilder(TBuilder&& builder) :m_builder(std::move(builder)) 
	{
		m_root_offset = m_builder.template Add<uint32_t>(0);
	};

	template <typename T>
	TableBuilder& Add(std::string_view name, T value)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.template Add<T>(value);

		return FieldConstructor(name_offset, offset);
	}

	TableBuilder& AddString(std::string_view name, std::string_view str)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.AddString(str);

		return FieldConstructor(name_offset, offset);
	}

	template <typename T>
	TableBuilder& AddArray(std::string_view name, std::span<const T> arr)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.template AddArray<T>(arr);

		return FieldConstructor(name_offset, offset);
	}

	//initializer_list
	template <typename T>
	TableBuilder& AddArray(std::string_view name, std::initializer_list<T> arr)
	{
		return AddArray<T>(name, std::span<const T>(arr.begin(), arr.size()));
	}

	Buffer Finish()
	{
		std::size_t table_offset = m_builder.template Add<uint32_t>(m_fields.size());
		m_builder.template WriteAt<uint32_t>(m_root_offset, table_offset);

		for (std::size_t i = 0; i < m_fields.size(); ++i)
		{
			m_builder.template Add<uint32_t>(m_fields[i].name_offset);
			m_builder.template Add<uint32_t>(m_fields[i].value_offset);
		}
		return m_builder.Finish();
	}

private:
	TBuilder m_builder;

	struct FieldInfo
	{
		std::size_t name_offset;
		std::size_t value_offset;
	};

	std::vector<FieldInfo> m_fields;
	std::size_t m_root_offset;

	TableBuilder& FieldConstructor(std::size_t name_offset, std::size_t offset)
	{
		FieldInfo field;
		field.name_offset = name_offset;
		field.value_offset = offset;

		m_fields.push_back(field);
		return *this;
	}
};