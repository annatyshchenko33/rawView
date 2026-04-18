#include "Buffer.hpp"
#include "Builder.hpp"
#include "TableBuilder.hpp"
#include "View.hpp"

#include <iostream>

int main()
{
	Builder builder;
	TableBuilder tb(builder);

	tb.Add<int32_t>(21)
		.Add<float>(98.5)
		.AddString("Zero");

	auto [buf, table_offset] = tb.Finish();

	View viewer(buf);

	std::cout << viewer.ReadTable<int32_t>(table_offset, 0) << std::endl;
	std::cout << viewer.ReadTable<float>(table_offset, 1) << std::endl;
	std::cout << viewer.ReadTableString(table_offset, 2) << std::endl;
}