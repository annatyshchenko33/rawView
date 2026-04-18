#pragma once
#include <vector>
#include <span>
#include <functional>
#include <memory>
#include <variant>
#include <stdexcept>
#include <type_traits>

class Buffer 
{
public:

	struct Deleters
	{
		static constexpr auto Free = [](void* ptr) {std::free(ptr); };
		static constexpr auto DeleteArr = [](uint8_t* ptr) {delete[] ptr; };
		static constexpr auto NoOperation = [](uint8_t*) {};
	};

	Buffer() noexcept
		:m_buffer(std::in_place_type<StackVector>) {};

	Buffer(std::vector<uint8_t>&& vector) noexcept
		:m_buffer(std::in_place_type<StackVector>, std::move(vector)) {};

	Buffer(uint8_t* data, std::size_t size, std::function<void(uint8_t*)> deleter)
		:m_buffer(std::in_place_type<ControllPtr>, data, size, std::move(deleter))
	{
		if (size > 0 && data == nullptr)
		{
			throw std::invalid_argument("Null pointer");
		}
		if (!std::get<ControllPtr>(m_buffer).ptr)
		{
			throw std::invalid_argument("Need a valid deleter");
		}
	}

	Buffer(char* data, std::size_t size, std::function<void(char*)> deleter)
		:Buffer(reinterpret_cast<uint8_t*>(data), size, [char_deleter = std::move(deleter)](uint8_t* ptr_to_delete)
			{
				if (ptr_to_delete) 
				{ 
					char_deleter(reinterpret_cast<char*>(ptr_to_delete));
				}
			}) {};


	Buffer(void* data, std::size_t size, std::function<void(void*)> deleter)
		:Buffer(static_cast<uint8_t*>(data), size, [void_deleter = std::move(deleter)](uint8_t* ptr_to_delete)
			{
				if (ptr_to_delete)
				{
					void_deleter(reinterpret_cast<char*>(ptr_to_delete));
				}
			}) {
	};

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	Buffer(Buffer&&) noexcept = default;
	Buffer& operator=(Buffer&&) noexcept = default;

	std::size_t get_size() const noexcept
	{
		return std::visit([](auto&& value) -> std::size_t
			{
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, StackVector>)
				{
					return value.size();
				}
				else
				{
					return value.c_size;
				}
			}, m_buffer);
	}

	const uint8_t* get_ptr() const noexcept
	{
		return std::visit([](auto&& value) -> const uint8_t*
			{
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, StackVector>)
				{
					return value.data();
				}
				else
				{
					return value.ptr.get();
				}
			}, m_buffer);
	}
	
	std::span<const uint8_t> raw_bytes() const noexcept
	{
		return std::span<const uint8_t>(get_ptr(), get_size());	
	}

	bool is_empty() const noexcept
	{
		return get_size() == 0;
	}

	bool is_owned() const noexcept
	{
		return std::holds_alternative<StackVector>(m_buffer);
	}

private:
	using StackVector = std::vector<uint8_t>;

	struct ControllPtr
	{
		std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> ptr;
		std::size_t c_size = 0;

		ControllPtr(uint8_t* data, size_t size, std::function<void(uint8_t*)> deleter)
			:ptr(data, std::move(deleter)), c_size(size) { }

		ControllPtr() = default;

	};

	std::variant<StackVector, ControllPtr> m_buffer;
};