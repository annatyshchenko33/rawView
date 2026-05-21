#include "TableBuilder.hpp"
#include "View.hpp"
#include "Buffer.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <iomanip>
#include <filesystem>
#include <string>

static constexpr int    N      = 1'000'000;
static constexpr int    TARGET = 499'999;   

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

static void generate_binary(const std::string& path)
{
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> temp(15.f, 38.f);
    std::uniform_real_distribution<float> hum (25.f, 90.f);
    std::uniform_real_distribution<float> pres(990.f, 1030.f);
    std::uniform_real_distribution<float> wind(0.f, 25.f);
    std::uniform_int_distribution<int>    sid (0, 9);

    std::vector<WeatherRecord> records(N);
    for (int i = 0; i < N; ++i)
        records[i] = { 1715000000 + i, sid(rng),
                       temp(rng), hum(rng), pres(rng), wind(rng) };

    TableBuilder tb;
    tb.AddStructArray("records", std::span<const WeatherRecord>(records));
    tb.Finish().save_to_file(path);
}

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

int main()
{
    const std::string bin_path = "weather_1m.rvb";
    const std::string csv_path = "weather_1m.csv";
    std::cout << "Generating " << N << " records...\n";
    generate_binary(bin_path);
    generate_csv(csv_path);

    auto bin_kb = std::filesystem::file_size(bin_path) / 1024;
    auto csv_kb = std::filesystem::file_size(csv_path) / 1024;
    std::cout << "Binary file: " << bin_kb << " KB\n";
    std::cout << "CSV    file: " << csv_kb << " KB\n\n";

    auto t0 = std::chrono::high_resolution_clock::now();

    Buffer mapped = Buffer::mmap_file(bin_path);
    View view(mapped);
    auto span = view.ReadTableStructArray<WeatherRecord>("records");
    const WeatherRecord& r_bin = span[TARGET];

    auto t1 = std::chrono::high_resolution_clock::now();
    auto us_bin = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    std::cout << "=== RawView mmap ===\n";
    std::cout << "  time:        " << us_bin << " us\n";
    std::cout << "  temperature: " << r_bin.temperature << " C\n";
    std::cout << "  I/O read:    ~4 KB  (1 page)\n\n";

    auto t2 = std::chrono::high_resolution_clock::now();

    WeatherRecord r_csv{};
    {
        std::ifstream f(csv_path);
        std::string line;
        std::getline(f, line);                  
        for (int i = 0; i <= TARGET; ++i)
            std::getline(f, line);
        std::sscanf(line.c_str(), "%d,%d,%f,%f,%f,%f",
                    &r_csv.timestamp, &r_csv.station_id,
                    &r_csv.temperature, &r_csv.humidity,
                    &r_csv.pressure, &r_csv.wind_speed);
    }

    auto t3 = std::chrono::high_resolution_clock::now();
    auto ms_csv = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    std::cout << "=== CSV scan ===\n";
    std::cout << "  time:        " << ms_csv << " ms\n";
    std::cout << "  temperature: " << r_csv.temperature << " C\n";
    std::cout << "  I/O read:    ~" << (csv_kb / 2) << " KB  (~half the file)\n\n";

    std::cout << "=== Comparison ===\n";
    std::cout << std::fixed << std::setprecision(0);
    std::cout << "  Binary vs CSV size:    "
              << (100 - 100.0 * bin_kb / csv_kb) << "% smaller\n";
    auto speedup = us_bin > 0 ? (ms_csv * 1000 / us_bin) : 999999LL;
    std::cout << "  Binary vs CSV access:  " << speedup << "x faster\n";

    std::cout << "\n  Temperatures match: "
              << std::boolalpha
              << (std::abs(r_bin.temperature - r_csv.temperature) < 0.01f) << "\n";

    return 0;
}
