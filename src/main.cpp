#include "Buffer.hpp"
#include "Builder.hpp"
#include "TableBuilder.hpp"
#include "View.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "Protocol/BinaryProtocol.hpp"
#include "Protocol/JsonProtocol.hpp"



#include <iostream>

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
}