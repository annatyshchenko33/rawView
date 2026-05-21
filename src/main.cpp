#include "Buffer.hpp"
#include "Builder.hpp"
#include "TableBuilder.hpp"
#include "View.hpp"
#include "MutableView.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "Protocol/BinaryProtocol.hpp"
#include "Protocol/JsonProtocol.hpp"

#include <iostream>

template<typename Protocol>
Buffer serialize_record(std::string_view name, int32_t value)
{
	Serializer<Protocol> writer;
	writer.AddString("name", name);
	writer.Add<int32_t>("value", value);
	return writer.Finish();
}

int main()
{
	//option
	//Builder builder;
	//TableBuilder tb(builder);

	TableBuilder tb;

	std::vector<int32_t> test = { 23, 45, 65 };

	tb.Add<int32_t>("age", 21).AddString("name", "anya").AddArray<int32_t>("phone_nums", {27362783, 378293292}).AddArray<int32_t>("int_", test);

	Buffer buf = tb.Finish();

	View viewer(buf);

	//index search
	std::cout << viewer.ReadTable<int32_t>(0) << std::endl;
	std::cout << viewer.ReadTableString(1) << std::endl;
	auto phones = viewer.ReadTableArr<int32_t>(2);
	auto int_ = viewer.ReadTableArr<int32_t>(3);

	for (auto p : phones)
	{
		std::cout << p << std::endl;
	}

	for (auto i : int_)
	{
		std::cout << i << std::endl;
	}

	//name search
	std::cout << viewer.ReadTable<int32_t>("age") << std::endl;
	std::cout << viewer.ReadTableString("name") << std::endl;
	auto phones1 = viewer.ReadTableArr<int32_t>("phone_nums");
	auto int_1 = viewer.ReadTableArr<int32_t>("int_");

	for (auto p : phones1)
	{
		std::cout << p << std::endl;
	}

	for (auto i : int_1)
	{
		std::cout << i << std::endl;
	}

	//[]
	std::cout << viewer["age"].as<int32_t>() << std::endl;
	std::cout << viewer["name"].asString() << std::endl;

	auto phones2 = viewer["phone_nums"].asArray<int32_t>();
	for (auto p : phones2)
	{
		std::cout << p << std::endl;
	}

	///////////////////////////////////

	Serializer<BinaryProtocol> serialize;

	serialize.Add<int32_t>("age", 3243);

	Buffer buf2 = serialize.Finish();

	Deserializer<BinaryProtocol> deserialize(buf2);

	std::cout << deserialize["age"].as<int32_t>() << std::endl;

	//////////////////////JSON//////////////////////

	Serializer<JsonProtocol> serializer;

	serializer.Add<double>("price", 20.34);

	Buffer buf3 = serializer.Finish();

	std::cout << "JSON" << std::endl;
	std::string json(buf3.get_ptr(), buf3.get_ptr() + buf3.get_size());

	std::cout << json << "\n";

	Deserializer<JsonProtocol> deserializer(buf3);

	std::cout << deserializer["price"].as<double>() << std::endl;

	////// NESTED TABLES///////////

	Serializer<BinaryProtocol> address;
	address.Add<int32_t>("zip", 49000).AddString("city", "Dnipro").AddString("street", "Hoholya");

	TableBuilder person;
	person.AddString("name", "Ivan").Add<int32_t>("age", 25).AddTable("address", std::move(address));

	Buffer buf4 = person.Finish();

	View person_view(buf4);

	std::cout << "\n--- nested tables ---\n";
	std::cout << person_view.ReadTableString("name") << "\n";
	std::cout << person_view.ReadTable<int32_t>("age") << "\n";

	View addr_view = person_view["address"].asTable();

	std::cout << addr_view.ReadTable<int32_t>("zip") << "\n";
	std::cout << addr_view.ReadTableString("city") << "\n";
	std::cout << addr_view.ReadTableString("street") << "\n";

	//borrowed data
	std::vector<uint8_t> raw = { 1, 2, 3, 4, 5 };
	Buffer buf5 = Buffer::borrow(raw.data(), raw.size());

	//struct

	struct Point
	{
		float x, y, z;
	};

	struct Color
	{
		uint8_t r, g, b;
	};

	Serializer<BinaryProtocol> scene;

	scene.AddStruct("origin", Point{ .x = 1.0f, .y = 2.5f, .z = 0.0f });
	scene.AddStruct("bg_color", Color{ .r = 255, .g = 128, .b = 0 });

	std::vector<Point> vertices = {
		{0.0f,  1.0f, 0.0f},
		{-1.0f, -1.0f, 0.0f},
		{1.0f, -1.0f, 0.0f},
	};

	scene.AddStructArray("vertices", vertices);

	Buffer buf6 = scene.Finish();

	View v(buf6);

	std::cout << "\n--- raw structs ---\n";

	const auto& origin = v["origin"].asStruct<Point>();
	std::cout << "origin: " << origin.x << " " << origin.y << " " << origin.z << "\n";

	const auto& bg = v["bg_color"].asStruct<Color>();
	std::cout << "bg_color: " << (int)bg.r << " " << (int)bg.g << " " << (int)bg.b << "\n";

	auto verts = v["vertices"].asStructArray<Point>();
	std::cout << "vertices (" << verts.size() << "):\n";
	for (const auto& p : verts)
	{
		std::cout << "  " << p.x << " " << p.y << " " << p.z << "\n";
	}

	// --- string arrays ---
	TableBuilder movie;
	movie.AddString("title", "Inception")
		.AddStringArray("genres", { "sci-fi", "action", "thriller" })
		.AddStringArray("actors", { "DiCaprio", "Gordon-Levitt", "Page" });

	Buffer movie_buf = movie.Finish();
	View movie_view(movie_buf);

	std::cout << "\n--- string arrays ---\n";
	std::cout << movie_view.ReadTableString("title") << "\n";

	auto genres = movie_view["genres"].asStringArray();
	std::cout << "genres (" << genres.size() << "): ";
	for (auto g : genres)
		std::cout << g << " ";
	std::cout << "\n";

	auto actors = movie_view["actors"].asStringArray();
	std::cout << "actors (" << actors.size() << "): ";
	for (auto a : actors)
		std::cout << a << " ";
	std::cout << "\n";

	////// FILE I/O + MMAP ///////////

	TableBuilder sensor;
	sensor.AddString("device", "temp_sensor_01")
	      .Add<float>("value", 36.6f)
	      .Add<int32_t>("timestamp", 1715000000);

	Buffer sensor_buf = sensor.Finish();

	// save to disk
	sensor_buf.save_to_file("sensor.rvb");
	std::cout << "\n--- file i/o ---\n";
	std::cout << "saved " << sensor_buf.get_size() << " bytes to sensor.rvb\n";

	// simple reading (copy in memory)
	Buffer loaded = Buffer::from_file("sensor.rvb");
	View loaded_view(loaded);
	std::cout << "from_file: " << loaded_view.ReadTableString("device") << " = "
	          << loaded_view.ReadTable<float>("value") << "\n";

	// mmap — zero-copy, OS loads pages directly from the file
	Buffer mapped = Buffer::mmap_file("sensor.rvb");
	View mapped_view(mapped);
	std::cout << "mmap_file: " << mapped_view.ReadTableString("device") << " = "
	          << mapped_view.ReadTable<float>("value") << "\n";

	////// ZERO-COPY SLICE///////////

	std::cout << "\n--- zero-copy ---\n";

	Serializer<BinaryProtocol> inner;
	inner.AddString("city", "Dnipro").Add<int32_t>("zip", 49000);

	TableBuilder packet;
	packet.AddString("from", "node_A")
	      .Add<int32_t>("id", 42)
	      .AddTable("payload", std::move(inner));

	Buffer full_buf = packet.Finish();
	View full_view(full_buf);

	View payload_view = full_view["payload"].asTable();
	std::cout << "city: " << payload_view.ReadTableString("city") << "\n";
	std::cout << "zip:  " << payload_view.ReadTable<int32_t>("zip") << "\n";

	std::span<const uint8_t> all_bytes = full_view.raw_bytes();
	std::cout << "full buffer via span: " << all_bytes.size() << " bytes\n";

	Buffer head = full_buf.slice(0, 8);
	std::cout << "slice(0,8): " << head.get_size() << " bytes, ptr same="
	          << (head.get_ptr() == full_buf.get_ptr() ? "yes" : "no") << "\n";

	////// MUTABLE VIEW ///////////

	TableBuilder reading;
	reading.AddString("device", "temp_sensor_01")
	       .Add<float>("value", 36.6f)
	       .Add<int32_t>("status", 0);

	Buffer reading_buf = reading.Finish();

	std::cout << "\n--- MutableView ---\n";
	std::cout << "before:  value=" << View(reading_buf).ReadTable<float>("value")
	          << "  status=" << View(reading_buf).ReadTable<int32_t>("status") << "\n";

	MutableView mv(reading_buf);

	mv.Set<float>("value", 37.2f);
	mv.Set<int32_t>("status", 1);

	std::cout << "after:   value=" << mv.Get<float>("value")
	          << "  status=" << mv.Get<int32_t>("status") << "\n";
	std::cout << "device:  " << mv.GetString("device") << "\n";
	std::cout << "has 'value':  " << std::boolalpha << mv.has("value") << "\n";
	std::cout << "has 'weight': " << mv.has("weight") << "\n";

	std::cout << "keys: ";
	for (auto k : mv.keys())
		std::cout << k << " ";
	std::cout << "\n";

	// original buffer also changed — no copy was made
	std::cout << "View(buf) sees new value: "
	          << View(reading_buf).ReadTable<float>("value") << "\n";

	// iterator: simple fields
	TableBuilder movie2;
	movie2.AddString("title", "Inception")
		.AddString("title2", "Scream")
		.AddString("title3", "Euphoria");

	Buffer m_buf = movie2.Finish();
	View v_mov(m_buf);

	std::cout << "\n--- iterator: simple fields ---\n";
	for (auto [key, proxy] : v_mov)
		std::cout << key << " = " << proxy.asString() << "\n";

	// iterator: nested table 
	TableBuilder city;
	city.Add<int32_t>("zip", 49000).AddString("name", "Dnipro");

	TableBuilder person2;
	person2.AddString("name", "Olena")
	       .Add<int32_t>("age", 28)
	       .AddTable("city", std::move(city));

	Buffer person2_buf = person2.Finish();
	View person2_view(person2_buf);

	std::cout << "\n--- iterator: nested table ---\n";
	for (auto [key, proxy] : person2_view)
	{
		if (key == "city")
		{
			std::cout << key << ":\n";
			View sub = proxy.asTable();
			for (auto [sub_key, sub_proxy] : sub)
				std::cout << "  " << sub_key << " = " << sub_proxy.as<int32_t>() << "\n";
		}
		else
		{
			std::cout << key << "\n";
		}
	}

	// iterator: string array
	TableBuilder catalog;
	catalog.AddString("title", "Dune")
	       .AddStringArray("tags", { "sci-fi", "epic", "drama" })
	       .Add<int32_t>("year", 2021);

	Buffer catalog_buf = catalog.Finish();
	View catalog_view(catalog_buf);

	std::cout << "\n--- iterator: string array ---\n";
	for (auto [key, proxy] : catalog_view)
	{
		if (key == "tags")
		{
			std::cout << key << ": ";
			for (auto tag : proxy.asStringArray())
				std::cout << tag << " ";
			std::cout << "\n";
		}
		else
		{
			std::cout << key << "\n";
		}
	}

	//mutable sub-view
	TableBuilder city2;
	city2.Add<int32_t>("zip", 49000).AddString("name", "Dnipro");

	TableBuilder person12;
	person12.AddString("name", "Olena").AddTable("city", std::move(city2));

	Buffer buf_sub_mv = person12.Finish();
	MutableView mv3(buf_sub_mv);

	MutableView city_mv = mv3.GetSubMutable("city");
	city_mv.Set<int32_t>("zip", 50000);

	View v_sub_mv(buf_sub_mv);
	View city_v = v_sub_mv["city"].asTable();
	std::cout << city_v.ReadTable<int32_t>("zip") << std::endl;

	////
	 // виклик для бінарного формату:
	Buffer bin = serialize_record<BinaryProtocol>("sensor_1", 42);
	// виклик для JSON:
	Buffer json_buf = serialize_record<JsonProtocol>("sensor_1", 42);
}