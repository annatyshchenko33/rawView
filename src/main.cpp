#include "Buffer.hpp"
#include "Builder.hpp"
#include "TableBuilder.hpp"
#include "View.hpp"

#include <iostream>

int main()
{
	Builder builder;
	TableBuilder tb(builder);

	tb.Add<int32_t>("age", 21).AddString("name", "anya");

	auto [buf, table_offset] = tb.Finish();

	View viewer(buf);

	//index search
	std::cout << viewer.ReadTable<int32_t>(table_offset, 0) << std::endl;
	std::cout << viewer.ReadTableString(table_offset, 1) << std::endl;

	//name search
	std::cout << viewer.ReadTable<int32_t>(table_offset, "age") << std::endl;
	std::cout << viewer.ReadTableString(table_offset, "name") << std::endl;
}