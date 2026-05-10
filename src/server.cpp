#include "Transport/SocketTransport.hpp"
#include "Deserializer.hpp"
#include "Protocol/BinaryProtocol.hpp"
#include <iostream>

int main()
{
	auto listener = SocketTransport::listen(9000);
	std::cout << "listening on :9000\n";

	auto conn = listener.accept();
	std::cout << "client connected\n";

	Buffer buf = conn.recv();

	Deserializer<BinaryProtocol> d(buf);
	std::cout << "name:  " << d["name"].asString() << "\n";
	std::cout << "age:   " << d["age"].as<int32_t>() << "\n";
	std::cout << "score: " << d["score"].as<float>() << "\n";
}