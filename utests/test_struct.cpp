#include <gtest/gtest.h>
#include "TableBuilder.hpp"
#include "View.hpp"

struct Point {
    float x, y, z;
};

struct Aligned8 {
    double value;
    int32_t id;
};

TEST(StructTest, ReadTableStructByName)
{
    TableBuilder tb;
    tb.AddStruct<Point>("pos", {1.0f, 2.0f, 3.0f});
    Buffer buf = tb.Finish();

    View view(buf);
    const Point& p = view.ReadTableStruct<Point>("pos");
    EXPECT_FLOAT_EQ(p.x, 1.0f);
    EXPECT_FLOAT_EQ(p.y, 2.0f);
    EXPECT_FLOAT_EQ(p.z, 3.0f);
}

TEST(StructTest, ReadTableStructByIndex)
{
    TableBuilder tb;
    tb.AddStruct<Point>("pos", {4.0f, 5.0f, 6.0f});
    Buffer buf = tb.Finish();

    View view(buf);
    const Point& p = view.ReadTableStruct<Point>(0);
    EXPECT_FLOAT_EQ(p.x, 4.0f);
    EXPECT_FLOAT_EQ(p.y, 5.0f);
    EXPECT_FLOAT_EQ(p.z, 6.0f);
}

TEST(StructTest, ReadTableStructByNameAndIndexSameResult)
{
    TableBuilder tb;
    tb.AddStruct<Point>("pos", {7.0f, 8.0f, 9.0f});
    Buffer buf = tb.Finish();

    View view(buf);
    const Point& by_name = view.ReadTableStruct<Point>("pos");
    const Point& by_index = view.ReadTableStruct<Point>(0);
    EXPECT_FLOAT_EQ(by_name.x, by_index.x);
    EXPECT_FLOAT_EQ(by_name.y, by_index.y);
    EXPECT_FLOAT_EQ(by_name.z, by_index.z);
}

TEST(StructTest, ReadTableStructInvalidNameThrows)
{
    TableBuilder tb;
    tb.AddStruct<Point>("pos", {1.0f, 2.0f, 3.0f});
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_THROW(view.ReadTableStruct<Point>("nonexistent"), std::out_of_range);
}

TEST(StructTest, ReadTableStructInvalidIndexThrows)
{
    TableBuilder tb;
    tb.AddStruct<Point>("pos", {1.0f, 2.0f, 3.0f});
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_THROW(view.ReadTableStruct<Point>(999), std::out_of_range);
}

TEST(StructTest, ReadTableStructMixedWithOtherFields)
{
    TableBuilder tb;
    tb.Add<int32_t>("id", 42)
      .AddStruct<Point>("pos", {1.0f, 2.0f, 3.0f})
      .AddString("name", "origin");
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_EQ(view.ReadTable<int32_t>("id"), 42);
    EXPECT_FLOAT_EQ(view.ReadTableStruct<Point>("pos").x, 1.0f);
    EXPECT_EQ(view.ReadTableString("name"), "origin");
}

TEST(StructTest, FieldProxyAsStruct)
{
    TableBuilder tb;
    tb.AddStruct<Point>("pos", {1.0f, 2.0f, 3.0f});
    Buffer buf = tb.Finish();

    View view(buf);
    const Point& p = view["pos"].asStruct<Point>();
    EXPECT_FLOAT_EQ(p.x, 1.0f);
    EXPECT_FLOAT_EQ(p.y, 2.0f);
    EXPECT_FLOAT_EQ(p.z, 3.0f);
}

TEST(StructArrayTest, ReadTableStructArrayByName)
{
    std::vector<Point> points = {{1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}, {7.f, 8.f, 9.f}};

    TableBuilder tb;
    tb.AddStructArray<Point>("points", std::span<const Point>(points));
    Buffer buf = tb.Finish();

    View view(buf);
    auto span = view.ReadTableStructArray<Point>("points");

    ASSERT_EQ(span.size(), 3u);
    EXPECT_FLOAT_EQ(span[0].x, 1.f);
    EXPECT_FLOAT_EQ(span[1].y, 5.f);
    EXPECT_FLOAT_EQ(span[2].z, 9.f);
}

TEST(StructArrayTest, ReadTableStructArrayByIndex)
{
    std::vector<Point> points = {{1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}};

    TableBuilder tb;
    tb.AddStructArray<Point>("points", std::span<const Point>(points));
    Buffer buf = tb.Finish();

    View view(buf);
    auto span = view.ReadTableStructArray<Point>(0);

    ASSERT_EQ(span.size(), 2u);
    EXPECT_FLOAT_EQ(span[0].x, 1.f);
    EXPECT_FLOAT_EQ(span[1].x, 4.f);
}

TEST(StructArrayTest, EmptyStructArray)
{
    std::vector<Point> points;

    TableBuilder tb;
    tb.AddStructArray<Point>("points", std::span<const Point>(points));
    Buffer buf = tb.Finish();

    View view(buf);
    auto span = view.ReadTableStructArray<Point>("points");
    EXPECT_EQ(span.size(), 0u);
}

TEST(StructArrayTest, ReadTableStructArrayInvalidNameThrows)
{
    std::vector<Point> points = {{1.f, 2.f, 3.f}};

    TableBuilder tb;
    tb.AddStructArray<Point>("points", std::span<const Point>(points));
    Buffer buf = tb.Finish();

    View view(buf);
    EXPECT_THROW(view.ReadTableStructArray<Point>("nonexistent"), std::out_of_range);
}

TEST(StructArrayTest, FieldProxyAsStructArray)
{
    std::vector<Point> points = {{1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}};

    TableBuilder tb;
    tb.AddStructArray<Point>("points", std::span<const Point>(points));
    Buffer buf = tb.Finish();

    View view(buf);
    auto span = view["points"].asStructArray<Point>();

    ASSERT_EQ(span.size(), 2u);
    EXPECT_FLOAT_EQ(span[0].x, 1.f);
    EXPECT_FLOAT_EQ(span[1].z, 6.f);
}

TEST(StructArrayTest, Aligned8StructArray)
{
    std::vector<Aligned8> items = {{3.14, 1}, {2.71, 2}};

    TableBuilder tb;
    tb.AddStructArray<Aligned8>("data", std::span<const Aligned8>(items));
    Buffer buf = tb.Finish();

    View view(buf);
    auto span = view.ReadTableStructArray<Aligned8>("data");

    ASSERT_EQ(span.size(), 2u);
    EXPECT_DOUBLE_EQ(span[0].value, 3.14);
    EXPECT_EQ(span[0].id, 1);
    EXPECT_DOUBLE_EQ(span[1].value, 2.71);
    EXPECT_EQ(span[1].id, 2);
}
