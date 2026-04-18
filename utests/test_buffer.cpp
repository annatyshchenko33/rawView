#include <gtest/gtest.h>

#include "Buffer.hpp"

TEST(BufferTest, EmptyByDefault)
{
    Buffer buf;
    EXPECT_TRUE(buf.is_empty());
    EXPECT_EQ(buf.get_size(), 0);
}

TEST(BufferTest, FromVector)
{
    std::vector<uint8_t> data = { 1, 2, 3, 4 };
    Buffer buf(std::move(data));
    EXPECT_FALSE(buf.is_empty());
    EXPECT_EQ(buf.get_size(), 4);
    EXPECT_TRUE(buf.is_owned());
}

TEST(BufferTest, FromRawPtr)
{
    uint8_t* ptr = new uint8_t[4]{ 1, 2, 3, 4 };
    Buffer buf(ptr, 4, Buffer::Deleters::DeleteArr);
    EXPECT_EQ(buf.get_size(), 4);
    EXPECT_FALSE(buf.is_owned());
}

TEST(BufferTest, NullPtrThrows)
{
    EXPECT_THROW(
        Buffer(nullptr, 4, Buffer::Deleters::DeleteArr),
        std::invalid_argument
    );
}