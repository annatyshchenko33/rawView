#include "Buffer.hpp"
#include "Builder.hpp"
#include "TableBuilder.hpp"
#include "View.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "Protocol/BinaryProtocol.hpp"

#include <iostream>

int main()
{
	//option
	//Builder builder;
	//TableBuilder tb(builder);

	TableBuilder tb;

	std::vector<int32_t> test = { 23, 45, 65 };

	tb.Add<int32_t>("age", 21).AddString("name", "anya").AddArray<int32_t>("phone_nums", {27362783, 378293292}).AddArray<int32_t>("int_", test);

	Buffer buf = tb.Finish();

	View viewer(buf);

	//index search
	std::cout << viewer.ReadTable<int32_t>(0) << std::endl;
	std::cout << viewer.ReadTableString(1) << std::endl;
	auto phones = viewer.ReadTableArr<int32_t>(2);
	auto int_ = viewer.ReadTableArr<int32_t>(3);

	for (auto p : phones)
	{
		std::cout << p << std::endl;
	}

	for (auto i : int_)
	{
		std::cout << i << std::endl;
	}

	//name search
	std::cout << viewer.ReadTable<int32_t>("age") << std::endl;
	std::cout << viewer.ReadTableString("name") << std::endl;
	auto phones1 = viewer.ReadTableArr<int32_t>("phone_nums");
	auto int_1 = viewer.ReadTableArr<int32_t>("int_");

	for (auto p : phones1)
	{
		std::cout << p << std::endl;
	}

	for (auto i : int_1)
	{
		std::cout << i << std::endl;
	}

	//[]
	std::cout << viewer["age"].as<int32_t>() << std::endl;
	std::cout << viewer["name"].asString() << std::endl;

	auto phones2 = viewer["phone_nums"].asArray<int32_t>();
	for (auto p : phones2)
	{
		std::cout << p << std::endl;
	}

	///////////////////////////////////

	Serializer<BinaryProtocol> serialize;

	serialize.Add<int32_t>("age", 3243);

	Buffer buf2 = serialize.Finish();

	Deserializer<BinaryProtocol> deserialize(buf2);

	std::cout << deserialize["age"].as<int32_t>() << std::endl;

}