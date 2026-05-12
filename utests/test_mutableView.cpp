#include <gtest/gtest.h>

#include "TableBuilder.hpp"
#include "View.hpp"
#include "MutableView.hpp"

TEST(MutableView, SetByName)
{
	TableBuilder tb;
	tb.Add<int32_t>("age", 32);

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	mv.Set<int32_t>("age", 21);
	EXPECT_EQ(View(buf).ReadTable<int32_t>("age"), 21);
}

TEST(MutableView, SetByIndex)
{
	TableBuilder tb;
	tb.Add<int32_t>("age", 32);

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	mv.Set<int32_t>(0, 21);
	EXPECT_EQ(mv.Get<int32_t>(0), 21);
}

TEST(MutableView, SetStringByName)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	mv.SetString("name", "Lviv");
	EXPECT_EQ(mv.GetString("name"), "Lviv");
}

TEST(MutableView, SetStringByIndex)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	mv.SetString(0, "Lviv");
	EXPECT_EQ(mv.GetString(0), "Lviv");
}

TEST(MutableView, SetStringWrongLengthThrows)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);
	
	EXPECT_THROW(mv.SetString(0, "Donetsk"), std::invalid_argument);
}

TEST(MutableView, SetMissingFieldThrows)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	EXPECT_THROW(mv.SetString("no_field", "Lviv"), std::runtime_error);
}

TEST(MutableView, SetIndexOutOfRangeThrows)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	EXPECT_THROW(mv.SetString(99999, "Lviv"), std::out_of_range);
}

TEST(MutableView, GetByName)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	EXPECT_EQ(mv.GetString("name"), "Kyiv");
}

TEST(MutableView, HasExisting)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	EXPECT_TRUE(mv.has("name"));
}

TEST(MutableView, HasMissing)
{
	TableBuilder tb;
	tb.AddString("name", "Kyiv");

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	EXPECT_FALSE(mv.has("qr"));
}

TEST(MutableView, Keys)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	std::vector<std::string_view> exp_res = { "a", "b", "c" };

	EXPECT_EQ(mv.keys(), exp_res);
}

TEST(MutableView, OriginalBufferMutated)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	mv.Set<int32_t>("a", 1);
	View v(buf);

	EXPECT_EQ(v.ReadTable<int32_t>("a"), 1);
}

TEST(MutableView, AsViewReflectsChanges)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	mv.Set<int32_t>("a", 1);
	EXPECT_EQ(mv.AsView().ReadTable<int32_t>("a"), 1);
}

TEST(MutableView, MultipleFields)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	Buffer buf = tb.Finish();
	MutableView mv(buf);

	mv.Set<int32_t>("a", 1);
	mv.Set<int32_t>("b", 2);
	EXPECT_EQ(mv.AsView().ReadTable<int32_t>("c"), 0);
}

TEST(MutableView, BorrowedBufferThrows)
{
	std::vector<uint8_t> raw = { 1, 2, 3, 4, 5 };
	Buffer buf = Buffer::borrow(raw.data(), raw.size());

	EXPECT_THROW(MutableView mv(buf), std::runtime_error);
}

TEST(MutableView, GetSubMutableByName)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	TableBuilder tb2;
	tb2.Add<int32_t>("d", 1).AddTable("first", std::move(tb));

	Buffer buf = tb2.Finish();

	MutableView mv(buf);
	MutableView mv_first = mv.GetSubMutable("first");
	mv_first.Set<int32_t>("a", 1);

	View v(buf);
	View sub_view = v["first"].asTable();
	EXPECT_EQ(sub_view.ReadTable<int32_t>("a"), 1);
}

TEST(MutableView, GetSubMutableByIndex)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	TableBuilder tb2;
	tb2.Add<int32_t>("d", 1).AddTable("first", std::move(tb));

	Buffer buf = tb2.Finish();

	MutableView mv(buf);
	MutableView mv_first = mv.GetSubMutable(1);
	mv_first.Set<int32_t>("a", 1);

	View v(buf);
	View sub_view = v["first"].asTable();
	EXPECT_EQ(sub_view.ReadTable<int32_t>("a"), 1);
}

TEST(MutableView, GetSubMutableMissingThrows)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	TableBuilder tb2;
	tb2.Add<int32_t>("d", 1).AddTable("first", std::move(tb));

	Buffer buf = tb2.Finish();

	MutableView mv(buf);
	EXPECT_THROW(mv.GetSubMutable("nope"), std::runtime_error);
}

TEST(MutableView, SubMutableDoesNotAffectOtherFields)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	TableBuilder tb2;
	tb2.Add<int32_t>("d", 1).AddTable("first", std::move(tb));

	Buffer buf = tb2.Finish();

	MutableView mv(buf);
	MutableView mv_first = mv.GetSubMutable(1);
	mv_first.Set<int32_t>("a", 1);

	View v(buf);
	View sub_view = v["first"].asTable();
	EXPECT_EQ(v.ReadTable<int32_t>("d"), 1);
}

TEST(MutableView, DoubleNested)
{
	TableBuilder tb;
	tb.Add<int32_t>("a", 0).Add<int32_t>("b", 0).Add<int32_t>("c", 0);

	TableBuilder tb2;
	tb2.Add<int32_t>("d", 1).AddTable("first", std::move(tb));

	TableBuilder tb3;
	tb3.Add<int32_t>("e", 1).AddTable("second", std::move(tb2));

	Buffer buf = tb3.Finish();

	MutableView mv(buf);
	MutableView mv_second = mv.GetSubMutable("second");
	MutableView mv_first = mv_second.GetSubMutable("first");
	mv_second.Set<int32_t>("d", 2);
	mv_first.Set<int32_t>("c", 2);

	View v(buf);
	EXPECT_EQ(v["second"].asTable().ReadTable<int32_t>("d"), 2);
	EXPECT_EQ(v["second"].asTable()["first"].asTable().ReadTable<int32_t>("c"), 2);
}