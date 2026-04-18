#include <gtest/gtest.h>

#include "Builder.hpp"
#include "View.hpp"

TEST(ViewTest, ReadInt)
{
    Builder b;
    auto offset = b.Add<int32_t>(25);
    Buffer buf = b.Finish();

    View view(buf);
    EXPECT_EQ(view.Read<int32_t>(offset), 25);
}

TEST(ViewTest, ReadFloat)
{
    Builder b;
    auto offset = b.Add<float>(3.14f);
    Buffer buf = b.Finish();

    View view(buf);
    EXPECT_FLOAT_EQ(view.Read<float>(offset), 3.14f);
}

TEST(ViewTest, ReadString)
{
    Builder b;
    auto offset = b.AddString("Anya");
    Buffer buf = b.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadString(offset), "Anya");
}

TEST(ViewTest, ReadMultiple)
{
    Builder b;
    auto age_off = b.Add<int32_t>(25);
    auto name_off = b.AddString("Anya");
    Buffer buf = b.Finish();

    View view(buf);
    EXPECT_EQ(view.Read<int32_t>(age_off), 25);
    EXPECT_EQ(view.ReadString(name_off), "Anya");
}

TEST(ViewTest, OutOfRangeThrows)
{
    Builder b;
    b.Add<int32_t>(25);
    Buffer buf = b.Finish();

    View view(buf);
    EXPECT_THROW(view.Read<int32_t>(9999), std::out_of_range);
}