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

TEST(TableBuilderTest, NestedTable)
{
    TableBuilder inner;
    inner.Add<int32_t>("zip", 12345).AddString("city", "Kyiv");

    TableBuilder outer;
    outer.Add<int32_t>("age", 30).AddTable("address", std::move(inner));
    Buffer buf = outer.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadTable<int32_t>("age"), 30);

    View addr = view["address"].asTable();
    EXPECT_EQ(addr.ReadTable<int32_t>("zip"), 12345);
    EXPECT_EQ(addr.ReadTableString("city"), "Kyiv");
}

TEST(TableBuilderTest, NestedTableInvalidFieldThrows)
{
    TableBuilder inner;
    inner.Add<int32_t>("x", 1);

    TableBuilder outer;
    outer.AddTable("sub", std::move(inner));
    Buffer buf = outer.Finish();

    View view(buf);
    View sub = view["sub"].asTable();
    EXPECT_THROW(sub.ReadTable<int32_t>("nonexistent"), std::out_of_range);
}

TEST(TableBuilderTest, DeeplyNestedTable)
{
    TableBuilder level2;
    level2.AddString("street", "Khreshchatyk");

    TableBuilder level1;
    level1.Add<int32_t>("zip", 1001).AddTable("location", std::move(level2));

    TableBuilder root;
    root.AddString("name", "Bob").AddTable("address", std::move(level1));
    Buffer buf = root.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadTableString("name"), "Bob");

    View addr = view["address"].asTable();
    EXPECT_EQ(addr.ReadTable<int32_t>("zip"), 1001);

    View loc = addr["location"].asTable();
    EXPECT_EQ(loc.ReadTableString("street"), "Khreshchatyk");
}