#pragma once

#include <yyjson.h>

#include <string_view>
#include <string>
#include <span>
#include <vector>         
#include <cstdlib> 
#include <stdexcept>

#include "../Buffer.hpp" 

class JsonWriter
{
public:
	JsonWriter()
	{
		m_doc = yyjson_mut_doc_new(nullptr);
		m_root = yyjson_mut_obj(m_doc);
		yyjson_mut_doc_set_root(m_doc, m_root);
	};

	~JsonWriter()
	{
		yyjson_mut_doc_free(m_doc);
	};

	JsonWriter(const JsonWriter&) = delete;
	JsonWriter& operator=(const JsonWriter&) = delete;

	JsonWriter(JsonWriter&& other) noexcept : m_doc(other.m_doc), m_root(other.m_root)
	{
		other.m_doc = nullptr;
		other.m_root = nullptr;
	}

	JsonWriter& operator=(JsonWriter&& other) noexcept 
	{
		if (this != &other) 
		{
			yyjson_mut_doc_free(m_doc);
			m_doc = other.m_doc;
			m_root = other.m_root;
			other.m_doc = nullptr;
			other.m_root = nullptr;
		}
		return *this;
	};

	template<typename T>
	JsonWriter& Add(std::string_view name, T value)
	{
		yyjson_mut_val* key = yyjson_mut_strncpy(m_doc, name.data(), name.size());
		yyjson_mut_val * val = nullptr;

		if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>)
		{
			val = yyjson_mut_sint(m_doc, value);
		}
		else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>)
		{
			val = yyjson_mut_real(m_doc, value);
		}
		else if constexpr (std::is_same_v <T, bool>)
		{
			val = yyjson_mut_bool(m_doc, value);
		}

		if (val)
		{
			yyjson_mut_obj_add(m_root, key, val);
		}

		return *this;
	}

	JsonWriter& AddString(std::string_view name, std::string_view value)
	{
		yyjson_mut_val* key = yyjson_mut_strncpy(m_doc, name.data(), name.size());
		yyjson_mut_val * val = yyjson_mut_strncpy(m_doc, value.data(), value.size());
		yyjson_mut_obj_add(m_root, key, val);

		return *this;
	}

	template<typename T>
	JsonWriter& AddArray(std::string_view name, std::span<const T> arr)
	{
		yyjson_mut_val* json_arr = yyjson_mut_arr(m_doc);

		for (auto& elem : arr)
		{
			if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>)
			{
				yyjson_mut_arr_add_sint(m_doc, json_arr, elem);
			}
			else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>)
			{
				yyjson_mut_arr_add_real(m_doc, json_arr, elem);
			}
			else if constexpr (std::is_same_v <T, bool>)
			{
				yyjson_mut_arr_add_bool(m_doc, json_arr, elem);
			}
		}

		yyjson_mut_val* key = yyjson_mut_strncpy(m_doc, name.data(), name.size());
		yyjson_mut_obj_add(m_root, key, json_arr);
		return *this;
	}

	//initializer_list
	template<typename T>
	JsonWriter& AddArray(std::string_view name, std::initializer_list<T> il)
	{
		return AddArray<T>(name, std::span<const T>(il.begin(), il.size()));
	}

	Buffer Finish()
	{
		std::size_t len = 0;
		char* json_str = yyjson_mut_write(m_doc, 0, &len);
		if (!json_str)
		{
			throw std::runtime_error("yyjson write failed");
		}

		std::vector<uint8_t> bytes(json_str, json_str + len);
		free(json_str);

		yyjson_mut_doc_free(m_doc);
		m_doc = nullptr;
		m_root = nullptr;

		return Buffer(std::move(bytes));
	}

private:
	yyjson_mut_doc* m_doc;
	yyjson_mut_val* m_root;
};
