#include <benchmark/benchmark.h>

#include "TableBuilder.hpp"
#include "View.hpp"
#include "MutableView.hpp"
#include "Protocol/JsonWriter.hpp"
#include "Protocol/JsonReader.hpp"

#include <rfl.hpp>
#include <rfl/json.hpp>

#include <yyjson.h>
#include "Verifier.hpp"

#include <string>

static void BM_Serialize(benchmark::State& state)
{
    for (auto _ : state)
    {
        TableBuilder tb;
        tb.Add<int32_t>("age", 25)
            .AddString("name", "Alexandra")
            .Add<double>("score", 98.5);
        Buffer buf = tb.Finish();
        benchmark::DoNotOptimize(buf);
    }

    {
        TableBuilder tb;
        tb.Add<int32_t>("age", 25)
            .AddString("name", "Alexandra")
            .Add<double>("score", 98.5);
        Buffer buf = tb.Finish();
        state.counters["buffer_bytes"] = static_cast<double>(buf.get_size());
    }
}
BENCHMARK(BM_Serialize);

static void BM_JsonSerialize(benchmark::State& state)
{
    for (auto _ : state)
    {
        JsonWriter writer;
        writer.Add<int32_t>("age", 25)
            .AddString("name", "Alexandra")
            .Add<double>("score", 98.5);
        Buffer buf = writer.Finish();
        benchmark::DoNotOptimize(buf);
    }

    {
        JsonWriter writer;
        writer.Add<int32_t>("age", 25)
            .AddString("name", "Alexandra")
            .Add<double>("score", 98.5);
        Buffer buf = writer.Finish();
        state.counters["buffer_bytes"] = static_cast<double>(buf.get_size());
    }
}
BENCHMARK(BM_JsonSerialize);

static void BM_FieldAccessByName(benchmark::State& state)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();
    View view(buf);

    for (auto _ : state)
    {
        auto val = view.ReadTable<int32_t>("age");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_FieldAccessByName);

static void BM_JsonFieldAccessByName(benchmark::State& state)
{
    JsonWriter writer;
    writer.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = writer.Finish();
    JsonReader reader(buf);

    for (auto _ : state)
    {
        auto val = reader.Read<int32_t>("age");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_JsonFieldAccessByName);

static void BM_FieldAccessByIndex(benchmark::State& state)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();
    View view(buf);

    for (auto _ : state)
    {
        auto val = view.ReadTable<int32_t>(0);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_FieldAccessByIndex);

static constexpr int kArraySize = 1000;

static void BM_LargeArrayAccess(benchmark::State& state)
{
    std::vector<int32_t> data(kArraySize);
    for (int i = 0; i < kArraySize; ++i) data[i] = i;

    TableBuilder tb;
    tb.AddArray<int32_t>("values", std::span<const int32_t>(data));
    Buffer buf = tb.Finish();
    View view(buf);

    for (auto _ : state)
    {
        auto span = view.ReadTableArr<int32_t>("values");
        benchmark::DoNotOptimize(span);
    }
}
BENCHMARK(BM_LargeArrayAccess);

static void BM_JsonLargeArrayAccess(benchmark::State& state)
{
    std::vector<int32_t> data(kArraySize);
    for (int i = 0; i < kArraySize; ++i) data[i] = i;

    JsonWriter writer;
    writer.AddArray<int32_t>("values", std::span<const int32_t>(data));
    Buffer buf = writer.Finish();
    JsonReader reader(buf);

    for (auto _ : state)
    {
        auto vec = reader.ReadArr<int32_t>("values");
        benchmark::DoNotOptimize(vec);
    }
}
BENCHMARK(BM_JsonLargeArrayAccess);

static void BM_StringAccess(benchmark::State& state)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
        .AddString("name", "Bright stars shimmer softly above calm lakes. now!")
        .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();
    View view(buf);

    for (auto _ : state)
    {
        std::string_view sv = view.ReadTableString("name");
        benchmark::DoNotOptimize(sv);
    }
}
BENCHMARK(BM_StringAccess);

static void BM_JsonStringAccess(benchmark::State& state)
{
    JsonWriter writer;
    writer.Add<int32_t>("age", 25)
        .AddString("name", "Bright stars shimmer softly above calm lakes. now!")
        .Add<double>("score", 98.5);
    Buffer buf = writer.Finish();
    JsonReader reader(buf);

    for (auto _ : state)
    {
        std::string s = reader.ReadString("name");
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_JsonStringAccess);

static void BM_MutableSet(benchmark::State& state)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();

    for (auto _ : state)
    {
        MutableView mv(buf);
        mv.Set<int32_t>("age", 26);
        benchmark::DoNotOptimize(mv);
    }
}
BENCHMARK(BM_MutableSet);

static void BM_JsonRebuild(benchmark::State& state)
{
    for (auto _ : state)
    {
        JsonWriter writer;
        writer.Add<int32_t>("age", 26)
            .AddString("name", "Alexandra")
            .Add<double>("score", 98.5);
        Buffer buf = writer.Finish();
        benchmark::DoNotOptimize(buf);
    }
}
BENCHMARK(BM_JsonRebuild);

static void BM_FieldScaling(benchmark::State& state)
{
    int n = static_cast<int>(state.range(0));

    TableBuilder tb;
    for (int i = 0; i < n; ++i)
        tb.Add<int32_t>("field" + std::to_string(i), i);
    Buffer buf = tb.Finish();
    View view(buf);

    std::string last = "field" + std::to_string(n - 1);

    for (auto _ : state)
    {
        auto val = view.ReadTable<int32_t>(last);
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_FieldScaling)->Arg(5)->Arg(10)->Arg(20)->Arg(50);

struct Person {
    int32_t age;
    std::string name;
    double score;
};

static void BM_ReflectJsonSerialize(benchmark::State& state)
{
    Person p{25, "Alexandra", 98.5};
    for (auto _ : state)
    {
        auto s = rfl::json::write(p);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_ReflectJsonSerialize);

static void BM_ReflectJsonDeserialize(benchmark::State& state)
{
    Person p{25, "Alexandra", 98.5};
    std::string json = rfl::json::write(p);

    for (auto _ : state)
    {
        auto result = rfl::json::read<Person>(json).value();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ReflectJsonDeserialize);

static void BM_ReflectJsonSerialize_size(benchmark::State& state)
{
    Person p{25, "Alexandra", 98.5};
    for (auto _ : state)
    {
        auto s = rfl::json::write(p);
        benchmark::DoNotOptimize(s);
    }

    auto s = rfl::json::write(Person{25, "Alexandra", 98.5});
    state.counters["buffer_bytes"] = static_cast<double>(s.size());
}
BENCHMARK(BM_ReflectJsonSerialize_size);

static void BM_NestedTableAccess(benchmark::State& state)
{
    TableBuilder inner;
    inner.Add<int32_t>("zip", 12345).AddString("city", "Kyiv");

    TableBuilder outer;
    outer.Add<int32_t>("age", 30).AddTable("address", std::move(inner));
    Buffer buf = outer.Finish();
    View view(buf);

    for (auto _ : state)
    {
        View addr = view["address"].asTable();
        auto zip = addr.ReadTable<int32_t>("zip");
        benchmark::DoNotOptimize(zip);
    }
}
BENCHMARK(BM_NestedTableAccess);

static void BM_JsonNestedAccess(benchmark::State& state)
{
    const std::string json = R"({"age":30,"address":{"zip":12345,"city":"Kyiv"}})";
    yyjson_doc* doc = yyjson_read(json.c_str(), json.size(), 0);
    yyjson_val* root = yyjson_doc_get_root(doc);

    for (auto _ : state)
    {
        yyjson_val* addr = yyjson_obj_get(root, "address");
        yyjson_val* zip_val = yyjson_obj_get(addr, "zip");
        auto zip = static_cast<int32_t>(yyjson_get_sint(zip_val));
        benchmark::DoNotOptimize(zip);
    }

    yyjson_doc_free(doc);
}
BENCHMARK(BM_JsonNestedAccess);

static constexpr int kStrArraySize = 100;

static void BM_StringArrayAccess(benchmark::State& state)
{
    std::vector<std::string> strs(kStrArraySize);
    for (int i = 0; i < kStrArraySize; ++i)
        strs[i] = "tag_string_" + std::to_string(i);

    std::vector<std::string_view> views(kStrArraySize);
    for (int i = 0; i < kStrArraySize; ++i)
        views[i] = strs[i];

    TableBuilder tb;
    tb.AddStringArray("tags", std::span<const std::string_view>(views));
    Buffer buf = tb.Finish();
    View view(buf);

    for (auto _ : state)
    {
        StringArrayView sav = view.ReadTableStringArray("tags");
        benchmark::DoNotOptimize(sav);
    }
}
BENCHMARK(BM_StringArrayAccess);

static void BM_JsonStringArrayAccess(benchmark::State& state)
{
    std::string json = "{\"tags\":[";
    for (int i = 0; i < kStrArraySize; ++i)
    {
        if (i > 0) json += ",";
        json += "\"tag_string_" + std::to_string(i) + "\"";
    }
    json += "]}";

    yyjson_doc* doc = yyjson_read(json.c_str(), json.size(), 0);
    yyjson_val* root = yyjson_doc_get_root(doc);

    for (auto _ : state)
    {
        yyjson_val* arr = yyjson_obj_get(root, "tags");
        std::vector<std::string> result;
        result.reserve(yyjson_arr_size(arr));

        yyjson_arr_iter iter = yyjson_arr_iter_with(arr);
        yyjson_val* item;
        while ((item = yyjson_arr_iter_next(&iter)))
            result.emplace_back(yyjson_get_str(item), yyjson_get_len(item));

        benchmark::DoNotOptimize(result);
    }

    yyjson_doc_free(doc);
}
BENCHMARK(BM_JsonStringArrayAccess);


static void BM_Verify(benchmark::State& state)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();

    for (auto _ : state)
    {
        Verifier v(buf);
        auto ok = v.VerifyBuffer();
        benchmark::DoNotOptimize(ok);
    }
}
BENCHMARK(BM_Verify);

struct DataWithArray {
    std::vector<int32_t> values;
};

static void BM_ReflectJsonLargeArray(benchmark::State& state)
{
    DataWithArray d;
    d.values.resize(kArraySize);
    for (int i = 0; i < kArraySize; ++i) d.values[i] = i;
    std::string json = rfl::json::write(d);

    for (auto _ : state)
    {
        auto result = rfl::json::read<DataWithArray>(json).value();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ReflectJsonLargeArray);

static void BM_FullCost_RawView(benchmark::State& state)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();

    for (auto _ : state)
    {
        View view(buf);
        auto val = view.ReadTable<int32_t>("age");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_FullCost_RawView);

static void BM_FullCost_Json(benchmark::State& state)
{
    JsonWriter writer;
    writer.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = writer.Finish();

    for (auto _ : state)
    {
        JsonReader reader(buf);
        auto val = reader.Read<int32_t>("age");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_FullCost_Json);

static void BM_FullCost_ReflectCpp(benchmark::State& state)
{
    std::string json = rfl::json::write(Person{25, "Alexandra", 98.5});

    for (auto _ : state)
    {
        auto result = rfl::json::read<Person>(json).value();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FullCost_ReflectCpp);

static Buffer BuildLargeRawBuffer()
{
    TableBuilder tb;
    for (int i = 0; i < 49; ++i)
        tb.Add<int32_t>("field" + std::to_string(i), i);
    tb.Add<int32_t>("target", 999);
    return tb.Finish();
}

static Buffer BuildLargeJsonBuffer()
{
    JsonWriter writer;
    for (int i = 0; i < 49; ++i)
        writer.Add<int32_t>("field" + std::to_string(i), i);
    writer.Add<int32_t>("target", 999);
    return writer.Finish();
}

static void BM_SelectiveRead_RawView(benchmark::State& state)
{
    Buffer buf = BuildLargeRawBuffer();

    for (auto _ : state)
    {
        View view(buf);
        auto val = view.ReadTable<int32_t>("target");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_SelectiveRead_RawView);

static void BM_SelectiveRead_Json(benchmark::State& state)
{
    Buffer buf = BuildLargeJsonBuffer();

    for (auto _ : state)
    {
        JsonReader reader(buf);
        auto val = reader.Read<int32_t>("target");
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_SelectiveRead_Json);

static void BM_Throughput_RawView(benchmark::State& state)
{
    TableBuilder tb;
    tb.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = tb.Finish();
    const std::size_t buf_size = buf.get_size();

    for (auto _ : state)
    {
        View view(buf);
        auto val = view.ReadTable<int32_t>("age");
        benchmark::DoNotOptimize(val);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(buf_size));
}
BENCHMARK(BM_Throughput_RawView);

static void BM_Throughput_Json(benchmark::State& state)
{
    JsonWriter writer;
    writer.Add<int32_t>("age", 25)
        .AddString("name", "Alexandra")
        .Add<double>("score", 98.5);
    Buffer buf = writer.Finish();
    const std::size_t buf_size = buf.get_size();

    for (auto _ : state)
    {
        JsonReader reader(buf);
        auto val = reader.Read<int32_t>("age");
        benchmark::DoNotOptimize(val);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(buf_size));
}
BENCHMARK(BM_Throughput_Json);