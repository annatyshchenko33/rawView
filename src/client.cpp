#include "Transport/SocketTransport.hpp"
#include "Serializer.hpp"
#include "Protocol/BinaryProtocol.hpp"
#include <iostream>

int main()
{
    Serializer<BinaryProtocol> s;
    s.AddString("name", "Ivan")
        .Add<int32_t>("age", 25)
        .Add<float>("score", 98.6f);

    Buffer buf = s.Finish();

    auto conn = SocketTransport::connect("127.0.0.1", 9000);
    conn.send(buf);
    std::cout << "sent\n";
}