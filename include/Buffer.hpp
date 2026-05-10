#pragma once
#include <vector>
#include <span>
#include <functional>
#include <memory>
#include <variant>
#include <stdexcept>
#include <type_traits>
#include <string>
#include <fstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

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
				else if constexpr (std::is_same_v<T, ControllPtr>)
				{
					return value.c_size;
				}
				else
				{
					return value.size;
				}
			}, m_buffer);
	}

	static Buffer borrow(const uint8_t* ptr, std::size_t size) noexcept
	{
		return Buffer(BorrowedSpan{ ptr, size });
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
				else if constexpr (std::is_same_v<T, ControllPtr>)
				{
					return value.ptr.get();
				}
				else
				{
					return value.ptr;
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

	static Buffer from_file(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			throw std::runtime_error("cannot open file: " + path);
		}
			
		std::size_t size = static_cast<std::size_t>(file.tellg());
		file.seekg(0);

		std::vector<uint8_t> data(size);
		file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
		return Buffer(std::move(data));
	}

	void save_to_file(const std::string& path) const
	{
		std::ofstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("cannot create file: " + path);
		}
		file.write(reinterpret_cast<const char*>(get_ptr()), static_cast<std::streamsize>(get_size()));
	}

	static Buffer mmap_file(const std::string& path)
	{
#ifdef _WIN32
		HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
		                          nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			throw std::runtime_error("cannot open file: " + path);
		}
			
		LARGE_INTEGER file_size{};
		GetFileSizeEx(file, &file_size);
		std::size_t size = static_cast<std::size_t>(file_size.QuadPart);

		if (size == 0) 
		{ 
			CloseHandle(file); return Buffer{}; 
		}

		HANDLE mapping = CreateFileMapping(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (!mapping)
		{
			CloseHandle(file);
			throw std::runtime_error("CreateFileMapping failed: " + path);
		}

		void* ptr = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
		if (!ptr)
		{
			CloseHandle(mapping);
			CloseHandle(file);
			throw std::runtime_error("MapViewOfFile failed: " + path);
		}

		return Buffer(static_cast<uint8_t*>(ptr), size,
			[file, mapping](uint8_t* p)
			{
				UnmapViewOfFile(p);
				CloseHandle(mapping);
				CloseHandle(file);
			});
#else
		int fd = open(path.c_str(), O_RDONLY);
		if (fd < 0)
		{
			throw std::runtime_error("cannot open file: " + path);
		}
			
		struct stat st{};
		fstat(fd, &st);
		std::size_t size = static_cast<std::size_t>(st.st_size);

		if (size == 0)
		{ 
			close(fd); return Buffer{}; 
		}

		void* ptr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
		close(fd);

		if (ptr == MAP_FAILED)
		{
			throw std::runtime_error("mmap failed: " + path);
		}
			
		return Buffer(static_cast<uint8_t*>(ptr), size,
			[size](uint8_t* p) { munmap(p, size); });
#endif
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

	struct BorrowedSpan
	{
		const uint8_t* ptr;
		std::size_t size;
	};

	std::variant<StackVector, ControllPtr, BorrowedSpan> m_buffer;

	Buffer(BorrowedSpan span) noexcept: m_buffer(span) {};
};