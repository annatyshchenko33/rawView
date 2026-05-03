#pragma once

#include <yyjson.h>

#include <cstdint>
#include <span>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>

class JsonFieldProxy
{
public:
	JsonFieldProxy(yyjson_val* val_ptr) : m_val_ptr(val_ptr) {};

	template <typename T>
	const T as() const
	{
		if constexpr (std::is_same_v<T, int32_t>)
		{
			return yyjson_get_sint(m_val_ptr);
		}
		else if constexpr (std::is_same_v<T, int64_t>)
		{
			return yyjson_get_sint(m_val_ptr);
		}
		else if constexpr (std::is_same_v<T, double>)
		{
			return yyjson_get_real(m_val_ptr);
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			return static_cast<float>(yyjson_get_real(m_val_ptr));
		}
		else if constexpr (std::is_same_v <T, bool>)
		{
			return yyjson_get_bool(m_val_ptr);
		}
	}

	std::string asString()
	{
		return std::string(yyjson_get_str(m_val_ptr), yyjson_get_len(m_val_ptr));
	}

	std::string_view asStringView()
	{
		return std::string_view(yyjson_get_str(m_val_ptr), yyjson_get_len(m_val_ptr));
	}

	template<typename T>
	std::vector<T> asArray()
	{
		std::size_t array_size = yyjson_arr_size(m_val_ptr);

		std::vector<T> result;
		result.reserve(array_size);

		for (std::size_t i = 0; i < array_size; ++i)
		{
			yyjson_val* elem = yyjson_arr_get(m_val_ptr, i);
			result.push_back(JsonFieldProxy(elem).as<T>());
		}
		return result;
	}

private:
	yyjson_val* m_val_ptr;
};