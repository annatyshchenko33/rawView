#pragma once

#include "JsonReader.hpp"
#include "JsonWriter.hpp"

struct JsonProtocol
{
	using Writer = JsonWriter;
	using Reader = JsonReader;
};