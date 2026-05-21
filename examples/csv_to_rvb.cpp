#include "TableBuilder.hpp"
#include "View.hpp"
#include "Buffer.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <array>
#include <filesystem>

static constexpr int N = 100'000;

struct WeatherRecord
{
    int32_t timestamp;
    int32_t station_id;
    float   temperature;
    float   humidity;
    float   pressure;
    float   wind_speed;
};
static_assert(sizeof(WeatherRecord) == 24);

static void generate_csv(const std::string& path)
{
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> temp(15.f, 38.f);
    std::uniform_real_distribution<float> hum (25.f, 90.f);
    std::uniform_real_distribution<float> pres(990.f, 1030.f);
    std::uniform_real_distribution<float> wind(0.f, 25.f);
    std::uniform_int_distribution<int>    sid (0, 9);

    std::ofstream f(path);
    f << "timestamp,station_id,temperature,humidity,pressure,wind_speed\n";
    f << std::fixed << std::setprecision(2);
    for (int i = 0; i < N; ++i)
        f << (1715000000 + i) << ',' << sid(rng) << ','
          << temp(rng) << ',' << hum(rng) << ','
          << pres(rng) << ',' << wind(rng) << '\n';
}

static std::vector<WeatherRecord> parse_csv(const std::string& path)
{
    std::vector<WeatherRecord> records;
    records.reserve(N);
    std::ifstream f(path);
    std::string line;
    std::getline(f, line);              
    while (std::getline(f, line))
    {
        WeatherRecord r{};
        std::sscanf(line.c_str(), "%d,%d,%f,%f,%f,%f",
                    &r.timestamp, &r.station_id,
                    &r.temperature, &r.humidity,
                    &r.pressure, &r.wind_speed);
        records.push_back(r);
    }
    return records;
}

static void aggregate(std::span<const WeatherRecord> span)
{
    std::array<double,  10> sum{};
    std::array<int32_t, 10> cnt{};
    for (const auto& r : span)
    {
        sum[r.station_id] += r.temperature;
        ++cnt[r.station_id];
    }
    std::cout << std::fixed << std::setprecision(2);
    for (int s = 0; s < 10; ++s)
        std::cout << "  WS_" << std::setw(2) << std::setfill('0') << s
                  << ": " << (sum[s] / cnt[s]) << " C"
                  << "  (" << cnt[s] << " readings)\n";
}

int main()
{
    const std::string csv_path = "weather_100k.csv";
    const std::string bin_path = "weather_100k.rvb";

    std::cout << "Generating " << N << "-row CSV...\n";
    generate_csv(csv_path);
    auto csv_kb = std::filesystem::file_size(csv_path) / 1024;
    std::cout << "CSV  file: " << csv_kb << " KB\n\n";

    auto t0 = std::chrono::high_resolution_clock::now();
    auto records = parse_csv(csv_path);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms_csv = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "CSV load (parse " << records.size() << " rows): "
              << ms_csv << " ms\n\n";

    {
        TableBuilder tb;
        tb.AddStructArray("records",
                          std::span<const WeatherRecord>(records));
        tb.Finish().save_to_file(bin_path);
    }

    auto bin_kb = std::filesystem::file_size(bin_path) / 1024;
    std::cout << "Binary file: " << bin_kb << " KB\n";
    std::cout << "Size reduction: "
              << (100 - 100 * bin_kb / csv_kb) << "% vs CSV\n\n";

    auto t2 = std::chrono::high_resolution_clock::now();
    Buffer mapped = Buffer::mmap_file(bin_path);
    View   view(mapped);
    auto   span = view.ReadTableStructArray<WeatherRecord>("records");
    auto t3 = std::chrono::high_resolution_clock::now();
    auto us_bin = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    std::cout << "Binary load (mmap " << span.size() << " records): "
              << us_bin << " us\n\n";

    std::cout << "=== Mean temperature per station (via span, no copy) ===\n";
    aggregate(span);

    std::cout << "\n=== Summary ===\n";
    std::cout << "  CSV  size:   " << csv_kb  << " KB\n";
    std::cout << "  Binary size: " << bin_kb  << " KB  ("
              << (100 - 100 * bin_kb / csv_kb) << "% smaller)\n";
    std::cout << "  CSV  load:   " << ms_csv  << " ms\n";
    auto speedup = us_bin > 0 ? (ms_csv * 1000 / us_bin) : 999999LL;
    std::cout << "  Binary load: " << us_bin  << " us  (~"
              << speedup << "x faster)\n";

    return 0;
}
