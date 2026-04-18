#include "Buffer.hpp"
#include "Builder.hpp"
#include "View.hpp"
#include <iostream>

int main()
{
	Builder builder;

	auto age_offset = builder.Add <int32_t> (21);
	auto name_offset = builder.AddString("Anya");

	Buffer buf = builder.Finish();

	View viewer(buf);

	int32_t age = viewer.Read<uint32_t>(age_offset);
	std::string_view name = viewer.ReadString(name_offset);

	std::cout << "age: " << age << "\n";
	std::cout << "name: " << name << "\n";
}