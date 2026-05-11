#include <gtest/gtest.h>

#include "Builder.hpp"
#include "TableBuilder.hpp"
#include "Verifier.hpp"

TEST(Verifier, ValidBufferPasses)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 32).Add<float>("weight", 58.5);

    Buffer buf = tb.Finish();
    Verifier verify(buf);
   
    EXPECT_TRUE(verify.VerifyBuffer());
}

TEST(Verifier, EmptyBufferFails)
{
    Buffer buf;
    Verifier verify(buf);

    EXPECT_FALSE(verify.VerifyBuffer());
}

TEST(Verifier, EmptyTablePasses)
{
    TableBuilder tb;
    Buffer buf = tb.Finish();
    Verifier verify(buf);

    EXPECT_TRUE(verify.VerifyBuffer());
}

TEST(Verifier, CorruptedRootOffsetFails)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 32).Add<float>("weight", 58.5);

    Buffer buf = tb.Finish();

    std::vector<uint8_t> corrupted(buf.raw_bytes().begin(), buf.raw_bytes().end());

    *reinterpret_cast<uint32_t*>(corrupted.data()) = 0xFFFFFFFF;
    
    Buffer buf2(std::move(corrupted));

    Verifier verify(buf2);

    EXPECT_FALSE(verify.VerifyBuffer());
}

TEST(Verifier, HugeFieldCountFails)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 32).Add<float>("weight", 58.5);

    Buffer buf = tb.Finish();

    std::vector<uint8_t> corrupted(buf.raw_bytes().begin(), buf.raw_bytes().end());

    uint32_t root_offset = *reinterpret_cast<const uint32_t*>(corrupted.data());
    *reinterpret_cast<uint32_t*>(corrupted.data() + root_offset) = 0xFFFFFFFF;

    Buffer buf2(std::move(corrupted));

    Verifier verify(buf2);

    EXPECT_FALSE(verify.VerifyBuffer());
}

TEST(Verifier, VerifyStringValid)
{
    Builder b;
    auto offset = b.AddString("hello");
    Buffer buf = b.Finish();

    Verifier verify(buf);

    EXPECT_TRUE(verify.VerifyString(offset));
}

TEST(Verifier, VerifyStringEmptyBuffer)
{
    Buffer buf;
    Verifier verify(buf);

    EXPECT_FALSE(verify.VerifyString(0));
}

TEST(Verifier, VerifyStringTruncated)
{
    Builder b;
    b.AddString("hello");
    Buffer buf = b.Finish();

    Buffer truncated = buf.slice(0, sizeof(uint32_t));

    Verifier verify(truncated);

    EXPECT_FALSE(verify.VerifyString(0));
}

TEST(Verifier, ManyFieldsPass)
{
    TableBuilder tb;
    tb.Add<int32_t>("a", 1)
      .Add<float>("b", 2.0f)
      .AddString("c", "test")
      .Add<int64_t>("d", 100)
      .AddString("e", "another");
    Buffer buf = tb.Finish();

    Verifier verify(buf);

    EXPECT_TRUE(verify.VerifyBuffer());
}
