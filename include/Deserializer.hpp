#pragma once

#include "Protocol/BinaryReader.hpp"

template<typename Protocol>
using Deserializer = typename Protocol::Reader;