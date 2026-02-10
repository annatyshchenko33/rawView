#include "rawview/buffer.hpp"
#include <iostream>

int main()
{
	Buffer buf;
	int a = 5;
	const char* b = "anya";
	int c = 10;
	buf.write(a);
	buf.write(b);
	buf.write(c);
	buf.raw_bytes();

}