#pragma once

#include <yyjson.h>

#include <string>
#include <string_view>
#include <span>
#include <stdexcept>

#include "../Buffer.hpp"
#include "JsonFieldProxy.hpp"

class JsonReader
{
public:
	JsonReader(const Buffer& buf)
	{
		const uint8_t* data_ptr = buf.get_ptr();
		const std::size_t size = buf.get_size();

		m_doc = yyjson_read(reinterpret_cast<const char*>(data_ptr), size, 0);

		if (!m_doc)
		{
			throw std::runtime_error("Failed to parse JSON");
		}

		m_root = yyjson_doc_get_root(m_doc);
	}

	~JsonReader() 
	{
		yyjson_doc_free(m_doc);
	}

	JsonReader(const JsonReader&) = delete;
	JsonReader& operator=(const JsonReader&) = delete;

	JsonReader(JsonReader&& other) noexcept : m_doc(other.m_doc), m_root(other.m_root)
	{
		other.m_doc = nullptr;
		other.m_root = nullptr;
	}

	JsonReader& operator=(JsonReader&& other) noexcept
	{
		if (this != &other)
		{
			yyjson_doc_free(m_doc);
			m_doc = other.m_doc;
			m_root = other.m_root;
			other.m_doc = nullptr;
			other.m_root = nullptr;
		}
		return *this;
	};

	JsonFieldProxy operator[](std::string_view name)
	{
		yyjson_val* node = yyjson_obj_getn(m_root, name.data(), name.size());
		if (node == nullptr)
		{
			throw std::out_of_range("Out of range(JsonRead)");
		}
		return JsonFieldProxy(node);
	}

	template<typename T>
	T Read(std::string_view name)
	{
		return operator[](name).as<T>();
	}

	std::string ReadString(std::string_view name)
	{
		return operator[](name).asString();
	}

	template<typename T>
	std::vector<T> ReadArr(std::string_view name)
	{
		return operator[](name).asArray<T>();
	}

private:
	yyjson_doc* m_doc;
	yyjson_val* m_root;
};