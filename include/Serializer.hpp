#pragma once

#include "TableBuilder.hpp"

template<typename Protocol>
using Serializer = typename Protocol::Writer;