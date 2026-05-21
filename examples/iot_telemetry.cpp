#include "TableBuilder.hpp"
#include "View.hpp"
#include "MutableView.hpp"
#include "HashedView.hpp"
#include "Buffer.hpp"

#include <iostream>
static Buffer serialize_reading(std::string_view device_id,
                                 float temperature,
                                 float humidity,
                                 int32_t timestamp,
                                 int32_t status)
{
    TableBuilder tb;
    tb.AddString("device",    device_id)
      .Add<float>("temp",     temperature)
      .Add<float>("humidity", humidity)
      .Add<int32_t>("ts",     timestamp)
      .Add<int32_t>("status", status);  
    return tb.Finish();
}

int main()
{
    Buffer reading = serialize_reading(
        "sensor_boiler_01", 87.3f, 42.1f, 1715000000, 0);

    std::cout << "=== 1. Serialization ===\n";
    std::cout << "Packet size: " << reading.get_size() << " bytes\n";
    const std::string path = "boiler_01.rvb";
    reading.save_to_file(path);
    std::cout << "\n=== 2. Saved to \"" << path << "\" ===\n";

    Buffer mapped = Buffer::mmap_file(path);
    View v(mapped);

    std::cout << "\n=== 3. Read via mmap (zero-copy) ===\n";
    std::cout << "device:   " << v.ReadTableString("device")   << "\n";
    std::cout << "temp:     " << v.ReadTable<float>("temp")     << " C\n";
    std::cout << "humidity: " << v.ReadTable<float>("humidity") << " %\n";
    std::cout << "status:   " << v.ReadTable<int32_t>("status") << "\n";

    HashedView hv(mapped);
    float temp = hv.ReadTable<float>("temp");
    float humidity = hv.ReadTable<float>("humidity");
    // обидва звернення — O(1) без повторного сканування

    std::cout << "\n=== 4. HashedView (O(1) lookup) ===\n";
    std::cout << "temp:     " << hv.ReadTable<float>("temp")     << " C\n";
    std::cout << "humidity: " << hv.ReadTable<float>("humidity") << " %\n";
    std::cout << "device:   " << hv.ReadTableString("device")   << "\n";
    {
        Buffer writable = Buffer::from_file(path);
        MutableView mv(writable);

        std::cout << "\n=== 5. MutableView (in-place mutation) ===\n";
        std::cout << "status before: " << mv.Get<int32_t>("status") << "\n";
        mv.Set<int32_t>("status", 2);
        std::cout << "status after:  " << mv.Get<int32_t>("status") << "\n";
        writable.save_to_file(path);
    }

    View final_v(Buffer::from_file(path));
    std::cout << "\n=== 6. Verification ===\n";
    std::cout << "persisted status = " << final_v.ReadTable<int32_t>("status")
              << "  (2 = critical)\n";

    return 0;
}
