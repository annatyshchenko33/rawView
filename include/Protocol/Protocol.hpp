#pragma once

template<typename Protocol>
struct ProtocolTraits
{
    using Writer = typename Protocol::Writer;
    using Reader = typename Protocol::Reader;
};