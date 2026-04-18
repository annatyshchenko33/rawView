#include <gtest/gtest.h>

#include "Builder.hpp"

TEST(BuilderTest, AddInt)
{
    Builder b;
    b.Add<int32_t>(42);
    Buffer buf = b.Finish();
    EXPECT_FALSE(buf.is_empty());
    EXPECT_GE(buf.get_size(), sizeof(int32_t));
}

TEST(BuilderTest, AddMultiple)
{
    Builder b;
    b.Add<int32_t>(1);
    b.Add<float>(3.14f);
    b.Add<int64_t>(100);
    Buffer buf = b.Finish();
    EXPECT_FALSE(buf.is_empty());
}

TEST(BuilderTest, AddString)
{
    Builder b;
    b.AddString("hello");
    Buffer buf = b.Finish();
    EXPECT_GE(buf.get_size(), sizeof(uint32_t) + 5);
}