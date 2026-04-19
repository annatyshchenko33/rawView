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

	Buffer buf = tb.Finish();

	View viewer(buf);

	//index search
	std::cout << viewer.ReadTable<int32_t>(0) << std::endl;
	std::cout << viewer.ReadTableString(1) << std::endl;

	//name search
	std::cout << viewer.ReadTable<int32_t>("age") << std::endl;
	std::cout << viewer.ReadTableString("name") << std::endl;
}