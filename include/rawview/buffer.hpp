#pragma once
#include <vector>
#include <span>
#include <functional>
#include <memory>
#include <variant>

class Buffer 
{
public:

	struct deleters
	{
		static constexpr auto free = [](void* ptr) {std::free(ptr); };
		static constexpr auto del_arr = [](uint8_t* ptr) {delete[] ptr; };
		static constexpr auto noOperation = [](uint8_t*) {};
	};

	Buffer() noexcept
		:buffer(std::in_place_type<StackVector>) {};

	Buffer(std::vector<uint8_t>&& vector) noexcept
		:buffer(std::in_place_type<StackVector>, std::move(vector)) {};

	Buffer(uint8_t* data, std::size_t size, std::function<void(uint8_t*)> deleter)
		:buffer(std::in_place_type<ControllPtr>, data, size, std::move(deleter))
	{
		if (size > 0 && data == nullptr)
		{
			throw std::invalid_argument("Null pointer");
		}
		if (!std::get<ControllPtr>(buffer).ptr.get_deleter())
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

	}

	const uint8_t get_ptr() const noexcept
	{

	}
	
	std::span<const uint8_t> raw_bytes() const noexcept
	{

	}

	bool is_empty() const noexcept
	{

	}

	bool is_owned() const noexcept
	{

	}

private:
	using StackVector = std::vector<uint8_t>;

	struct ControllPtr
	{
		std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> ptr;
		std::size_t count = 0;

		ControllPtr(uint8_t* data, size_t size, std::function<void(uint8_t*)> deleter)
			:ptr(data, std::move(deleter)), count(size) { }

		ControllPtr() = default;

	};

	std::variant<StackVector, ControllPtr> buffer;
};