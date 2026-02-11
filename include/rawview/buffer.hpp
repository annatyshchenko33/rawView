#pragma once
#include <vector>
#include <span>
#include <functional>

class Buffer 
{
public:

	struct deleters
	{
		static constexpr auto free = [](void* ptr) {std::free(ptr); };
		static constexpr auto del_arr = [](uint8_t* ptr) {delete[] ptr; };
		static constexpr auto noOperation = [](uint8_t*) {};
	};

	Buffer() = default;

	//std::size_t get_size() const
	//{
	//	return buffer.size();
	//}

	//std::span<const uint8_t> raw_bytes()
	//{
	//	return std::span<const uint8_t>(buffer.data(), buffer.size());
	//}

	template<typename T>
	void write(const T& value)
	{

			auto alignment = alignof(T);
			align_offset(alignment);

			std::size_t  needed_size = write_offset + sizeof(T);
			if (needed_size > buffer.size())
			{
				resize(needed_size);
			}

			memcpy(buffer.data() + write_offset, &value, sizeof(T));
			write_offset += sizeof(T);
		
	}

	template<typename T>
	T read()
	{
		
			auto alignment = alignof(T);
			read_offset = (read_offset + alignment - 1) & ~(alignment - 1);

			if (read_offset + sizeof(T) > buffer.size())
				throw std::out_of_range("Buffer underflow");

			T value;

			memcpy(&value, buffer.data() + read_offset, sizeof(T));

			read_offset += sizeof(T);
			return value;
		

	}
	

private:
	using StackVector = std::vector<uint8_t>;
	//std::size_t write_offset = 0;
	//std::size_t read_offset = 0;

	//void align_offset(std::size_t alignment)
	//{
	//	write_offset = (write_offset + alignment - 1) & ~(alignment - 1);
	//}

	//void resize(std::size_t new_size)
	//{
	//	buffer.resize(new_size);
	//}

	struct ControllPtr
	{
		std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> ptr;
		std::size_t count = 0;

		ControllPtr(uint8_t data, size_t size, std::function<void(uint8_t*)> deleter)
		{

		}

	};
};