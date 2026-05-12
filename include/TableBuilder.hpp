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

	template <RawStruct T>
	TableBuilder& AddStruct(std::string_view name, T value)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.template AddStruct<T>(value);
		return FieldConstructor(name_offset, offset);
	}

	template <RawStruct T>
	TableBuilder& AddStructArray(std::string_view name, std::span<const T> arr)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.template AddStructArray<T>(arr);
		return FieldConstructor(name_offset, offset);
	}

	template <RawStruct T>
	TableBuilder& AddStructArray(std::string_view name, std::vector<T> const& arr)
	{
		return AddStructArray<T>(name, std::span<const T>(arr));
	}

	TableBuilder& AddTable(std::string_view name, TableBuilder<TBuilder>&& sub)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t embed_offset = m_builder.Embed(sub.m_builder, sizeof(uint32_t));

		const std::size_t adjustment = embed_offset - sizeof(uint32_t);

		for (std::size_t sub_pos : sub.m_desc_patches)
		{
			std::size_t our_pos = embed_offset + sub_pos - sizeof(uint32_t);
			uint32_t val = m_builder.template ReadAt<uint32_t>(our_pos);
			m_builder.template WriteAt<uint32_t>(our_pos, val + static_cast<uint32_t>(adjustment));
			m_desc_patches.push_back(our_pos);
		}

		std::size_t sub_desc_offset = m_builder.template Add<uint32_t>((uint32_t)sub.m_fields.size());
		for (auto& f : sub.m_fields)
		{
			std::size_t npos = m_builder.template Add<uint32_t>(static_cast<uint32_t>(f.name_offset + adjustment));
			std::size_t vpos = m_builder.template Add<uint32_t>(static_cast<uint32_t>(f.value_offset + adjustment));
			m_desc_patches.push_back(npos);
			m_desc_patches.push_back(vpos);
		}
		return FieldConstructor(name_offset, sub_desc_offset);
	}

	TableBuilder& AddTable(std::string_view name, const TableBuilder<TBuilder>& sub)
	{
		TableBuilder<TBuilder> copy = sub;
		return AddTable(name, std::move(copy));
	}

	TableBuilder& AddStringArray(std::string_view name, std::span<const std::string_view> arr)
	{
		std::size_t name_offset = m_builder.AddString(name);
		std::size_t offset = m_builder.AddStringArray(arr);
		return FieldConstructor(name_offset, offset);
	}

	TableBuilder& AddStringArray(std::string_view name, std::initializer_list<std::string_view> arr)
	{
		return AddStringArray(name, std::span<const std::string_view>(arr.begin(), arr.size()));
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
	std::vector<std::size_t> m_desc_patches;
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