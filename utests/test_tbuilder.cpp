#include <gtest/gtest.h>
#include "TableBuilder.hpp"
#include "View.hpp"

TEST(TableBuilderTest, ReadByName)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 21).AddString("name", "Anya");
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadTable<int32_t>("age"), 21);
    EXPECT_EQ(view.ReadTableString("name"), "Anya");
}

TEST(TableBuilderTest, ReadByIndex)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 21).AddString("name", "Anya");
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadTable<int32_t>(0), 21);
    EXPECT_EQ(view.ReadTableString(1), "Anya");
}

TEST(TableBuilderTest, InvalidNameThrows)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 21);
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_THROW(view.ReadTableString("nonexistent"), std::out_of_range);
}

TEST(TableBuilderTest, InvalidIndexThrows)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 21);
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_THROW(view.ReadTable<int32_t>(999), std::out_of_range);
}

TEST(TableBuilderTest, MultipleTypes)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 21)
        .Add<float>("score", 98.5f)
        .AddString("name", "Anya");
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadTable<int32_t>("age"), 21);
    EXPECT_FLOAT_EQ(view.ReadTable<float>("score"), 98.5f);
    EXPECT_EQ(view.ReadTableString("name"), "Anya");
}

TEST(TableBuilderTest, ReadByNameAndIndexSameResult)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 21).AddString("name", "Anya");
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadTable<int32_t>("age"), view.ReadTable<int32_t>(0));
    EXPECT_EQ(view.ReadTableString("name"), view.ReadTableString(1));
}

TEST(TableBuilderTest, EmptyTableThrows)
{
    TableBuilder tb;
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_THROW(view.ReadTable<int32_t>(0), std::out_of_range);
    EXPECT_THROW(view.ReadTableString("age"), std::out_of_range);
}