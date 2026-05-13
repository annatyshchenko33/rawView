#include <gtest/gtest.h>
#include "TableBuilder.hpp"
#include "HashedView.hpp"

TEST(HashedViewTest, ReadTableScalar)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25).Add<double>("score", 98.5);
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_EQ(hv.ReadTable<int32_t>("age"), 25);
    EXPECT_DOUBLE_EQ(hv.ReadTable<double>("score"), 98.5);
}

TEST(HashedViewTest, ReadTableString)
{
    TableBuilder tb;
    tb.AddString("name", "Alexandra");
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_EQ(hv.ReadTableString("name"), "Alexandra");
}

TEST(HashedViewTest, ReadTableArr)
{
    TableBuilder tb;
    tb.AddArray<int32_t>("vals", {1, 2, 3, 4, 5});
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    auto span = hv.ReadTableArr<int32_t>("vals");
    ASSERT_EQ(span.size(), 5u);
    EXPECT_EQ(span[2], 3);
}

TEST(HashedViewTest, OperatorBracket)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 42);
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_EQ(hv["age"].as<int32_t>(), 42);
}

TEST(HashedViewTest, Has)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25).AddString("name", "Anya");
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_TRUE(hv.has("age"));
    EXPECT_TRUE(hv.has("name"));
    EXPECT_FALSE(hv.has("nonexistent"));
}

TEST(HashedViewTest, InvalidNameThrows)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25);
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_THROW(hv.ReadTable<int32_t>("nonexistent"), std::out_of_range);
}

TEST(HashedViewTest, SameResultAsView)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
      .AddString("name", "Alexandra")
      .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();

    View view(buf);
    HashedView hv(buf);

    EXPECT_EQ(hv.ReadTable<int32_t>("age"), view.ReadTable<int32_t>("age"));
    EXPECT_EQ(hv.ReadTableString("name"), view.ReadTableString("name"));
    EXPECT_DOUBLE_EQ(hv.ReadTable<double>("score"), view.ReadTable<double>("score"));
}

TEST(HashedViewTest, ByIndexDelegatesToView)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25).AddString("name", "Alexandra");
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_EQ(hv.ReadTable<int32_t>(0), 25);
    EXPECT_EQ(hv.ReadTableString(1), "Alexandra");
}

TEST(HashedViewTest, ManyFields)
{
    TableBuilder tb;
    for (int i = 0; i < 50; ++i)
        tb.Add<int32_t>("field" + std::to_string(i), i);
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_EQ(hv.ReadTable<int32_t>("field0"), 0);
    EXPECT_EQ(hv.ReadTable<int32_t>("field25"), 25);
    EXPECT_EQ(hv.ReadTable<int32_t>("field49"), 49);
}

TEST(HashedViewTest, FieldCount)
{
    TableBuilder tb;
    tb.Add<int32_t>("a", 1).Add<int32_t>("b", 2).Add<int32_t>("c", 3);
    Buffer buf = tb.Finish();

    HashedView hv(buf);
    EXPECT_EQ(hv.field_count(), 3u);
}
