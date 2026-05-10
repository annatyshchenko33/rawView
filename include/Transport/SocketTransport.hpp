#pragma once

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using ssize_t = int;
    #define sock_close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define sock_close ::close
#endif

#include "../Buffer.hpp"

class SocketTransport
{
public:
	SocketTransport(SocketTransport&& o) noexcept : m_fd(o.m_fd) 
	{
		o.m_fd = -1;
	}

	SocketTransport& operator=(SocketTransport&& o) noexcept
	{
		if (this != &o)
		{
			if (m_fd >= 0)
			{
				sock_close(m_fd);
			}
			m_fd   = o.m_fd;
			o.m_fd = -1;
		}
		return *this;
	}

	SocketTransport(const SocketTransport&) = delete;
	SocketTransport& operator=(const SocketTransport&) = delete;

	~SocketTransport() 
	{ 
		if (m_fd >= 0)
		{
			sock_close(m_fd);
		} 
	}

	static SocketTransport connect(std::string_view host, uint16_t port)
	{
#ifdef _WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
		int fd = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
		if (fd < 0)
		{
			throw std::runtime_error("socket() failed");
		}

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		::inet_pton(AF_INET, host.data(), &addr.sin_addr);

		if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
		{
			sock_close(fd);
			throw std::runtime_error("connect() failed");
		}
		return SocketTransport(fd);
	}

	static SocketTransport listen(uint16_t port)
	{
#ifdef _WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
		int fd = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
		if (fd < 0)
		{
			throw std::runtime_error("socket() failed");
		}
	
		int opt = 1;
		::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port);

		if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
		{
			sock_close(fd);
			throw std::runtime_error("bind() failed");
		}
		::listen(fd, 8);
		return SocketTransport(fd);
	}

	SocketTransport accept()
	{
		int client_fd = ::accept(m_fd, nullptr, nullptr);
		if (client_fd < 0)
		{
			throw std::runtime_error("accept() failed");
		}
		return SocketTransport(client_fd);
	}

	void send(const Buffer& buf)
	{
		uint32_t size = static_cast<uint32_t>(buf.get_size());
		send_all(&size, sizeof(size));
		send_all(buf.get_ptr(), size);
	}

	Buffer recv()
	{
		uint32_t size = 0;
		recv_all(&size, sizeof(size));

		std::vector<uint8_t> data(size);
		recv_all(data.data(), size);
		return Buffer(std::move(data));
	}

	void send_raw(std::span<const uint8_t> bytes)
	{
		send_all(bytes.data(), bytes.size());
	}

	Buffer recv_into(std::vector<uint8_t>& preallocated)
	{
		uint32_t size = 0;
		recv_all(&size, sizeof(size));

		if (size > preallocated.size())
		{
			preallocated.resize(size);
		}
			
		recv_all(preallocated.data(), size);
		return Buffer::borrow(preallocated.data(), size);
	}

private:
	int m_fd = -1;
	explicit SocketTransport(int fd) : m_fd(fd) {}

	void send_all(const void* buf, std::size_t len)
	{
		const char* ptr = static_cast<const char*>(buf);
		while (len > 0)
		{
			ssize_t n = ::send(m_fd, ptr, len, 0);
			if (n <= 0)
			{
				throw std::runtime_error("send failed");
			}
			ptr += n; len -= n;
		}
	}

	void recv_all(void* buf, std::size_t len)
	{
		char* ptr = static_cast<char*>(buf);
		while (len > 0)
		{
			ssize_t n = ::recv(m_fd, ptr, len, 0);
			if (n <= 0)
			{
				throw std::runtime_error("connection closed");
			}
			ptr += n; len -= n;
		}
	}
};