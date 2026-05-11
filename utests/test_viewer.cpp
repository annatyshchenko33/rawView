#include <gtest/gtest.h>

#include "Builder.hpp"
#include "TableBuilder.hpp"
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

TEST(ViewTest, HasExistingField)
{
    TableBuilder  tb;
    tb.Add<int32_t>("age", 45);
    Buffer buf = tb.Finish();

    View viewer(buf);
    EXPECT_TRUE(viewer.has("age"));
}

TEST(ViewTest, HasMissingField)
{
    TableBuilder  tb;
    tb.Add<int32_t>("age", 45);
    Buffer buf = tb.Finish();

    View viewer(buf);
    EXPECT_FALSE(viewer.has("zxc"));
}

TEST(ViewTest, HasEmptyTable)
{
    TableBuilder  tb;
    Buffer buf = tb.Finish();

    View viewer(buf);
    EXPECT_FALSE(viewer.has("zxc"));
}

TEST(ViewTest, KeysOrder)
{
    TableBuilder  tb;
    tb.Add<int32_t>("age", 45).AddString("name", "Sasha").AddStringArray("friends", { "Anya", "Vlad", "Olya" });
    Buffer buf = tb.Finish();

    View viewer(buf);
    std::vector<std::string_view> exp_res = { "age", "name", "friends" };
    EXPECT_EQ(viewer.keys(), exp_res);
}

TEST(ViewTest, KeysEmpty)
{
    TableBuilder  tb;
    Buffer buf = tb.Finish();

    View viewer(buf);
    std::vector<std::string_view> exp_res = {};
    EXPECT_EQ(viewer.keys(), exp_res);
}

TEST(ViewTest, FieldCount)
{
    TableBuilder  tb;
    tb.Add<int32_t>("age", 45).AddString("name", "Sasha").AddStringArray("friends", { "Anya", "Vlad", "Olya" });
    Buffer buf = tb.Finish();

    View viewer(buf);
    EXPECT_EQ(viewer.field_count(), 3);
}
