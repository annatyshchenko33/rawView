#pragma once

#include "BinaryReader.hpp"
#include "TableBuilder.hpp"

struct BinaryProtocol
{
	using Writer = TableBuilder<Builder>;
	using Reader = BinaryReader;
};