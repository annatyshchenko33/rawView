# RawView

Бібліотека бінарної серіалізації для C++20 без генерації коду та сторонніх схем.

Дані зберігаються у компактному бінарному форматі з іменованими полями. Читання виконується без копіювання через `std::span` і `mmap`; числові поля доступні напряму через `reinterpret_cast`. Підтримуються два протоколи — бінарний і JSON — із єдиним інтерфейсом серіалізації.

---

## Зміст

- [Можливості](#можливості)
- [Вимоги та збірка](#вимоги-та-збірка)
- [Компоненти бібліотеки](#компоненти-бібліотеки)
- [Підтримувані типи](#підтримувані-типи)
- [Приклади використання](#приклади-використання)
- [Протоколи та мережа](#протоколи-та-мережа)
- [Управління буфером](#управління-буфером)
- [Результати бенчмарків](#результати-бенчмарків)
- [Структура проекту](#структура-проекту)

---

## Можливості

- **Без генерації коду** — поля визначаються рядковими іменами безпосередньо в коді
- **Zero-copy читання** — `View` та `HashedView` працюють поверх `std::span<const uint8_t>` без копіювання
- **mmap-підтримка** — `Buffer::mmap_file` відображає файл в адресний простір; OS завантажує лише потрібні сторінки
- **In-place мутація** — `MutableView` змінює числові поля безпосередньо в буфері без реалокації
- **O(1) доступ за іменем** — `HashedView` будує хеш-таблицю одноразово при конструюванні
- **Вкладені таблиці** — `AddTable` дозволяє будь-яку глибину вкладеності
- **Struct arrays** — суцільні масиви trivially-copyable структур через `AddStructArray`
- **Два протоколи** — `BinaryProtocol` (швидкість, компактність) і `JsonProtocol` (читабельність) з єдиним API
- **Мережевий транспорт** — `SocketTransport` для передачі `Buffer` через TCP
- **Верифікація** — `Verifier` перевіряє цілісність буфера без виключень

---

## Вимоги та збірка

- C++20 (concepts, span, ranges)
- CMake ≥ 3.20

```bash
cmake -B build
cmake --build build
```

Опційні цілі:

```bash
# тести
cmake --build build --target UTests

# бенчмарки
cmake -B build -DBUILD_BENCHMARKS=ON
cmake --build build

# приклади
cmake -B build -DBUILD_EXAMPLES=ON
cmake --build build
```

---

## Компоненти бібліотеки

| Клас | Заголовок | Призначення |
|---|---|---|
| `Builder` | `Builder.hpp` | Низькорівневий будівник байтового буфера |
| `TableBuilder` | `TableBuilder.hpp` | Будівник іменованих полів; повертає `Buffer` |
| `View` | `View.hpp` | Read-only доступ до буфера за іменем або індексом |
| `HashedView` | `HashedView.hpp` | Як `View`, але з O(1) пошуком за іменем |
| `MutableView` | `MutableView.hpp` | Read-write доступ; змінює поля in-place |
| `Buffer` | `Buffer.hpp` | Власник байтів; підтримує owned/borrowed/mmap режими |
| `FieldProxy` | `FieldProxy.hpp` | Проксі для синтаксису `view["key"].as<T>()` |
| `StringArrayView` | `StringArrayView.hpp` | Zero-copy ітерація масиву рядків |
| `Verifier` | `Verifier.hpp` | Перевірка структурної цілісності буфера |
| `Serializer<P>` | `Serializer.hpp` | Псевдонім для `Protocol::Writer` |
| `Deserializer<P>` | `Deserializer.hpp` | Псевдонім для `Protocol::Reader` |
| `SocketTransport` | `Transport/SocketTransport.hpp` | TCP send/recv для `Buffer` |

---

## Підтримувані типи

### Скалярні (`SupportedScalar` — всі арифметичні типи)

```cpp
tb.Add<int32_t>("age",   25);
tb.Add<float>  ("score", 98.5f);
tb.Add<double> ("pi",    3.14159);
tb.Add<uint8_t>("flag",  1);
```

### Рядки

```cpp
tb.AddString("name", "Олена");
```

### Масиви скалярів

```cpp
tb.AddArray<int32_t>("ids",    {1, 2, 3});
tb.AddArray<float>  ("values", std::span<const float>(vec));
```

### Масиви рядків

```cpp
tb.AddStringArray("tags", {"c++", "binary", "zero-copy"});
```

### Raw-структури (`RawStruct` — trivially-copyable, standard-layout)

```cpp
struct Point { float x, y, z; };

tb.AddStruct("origin", Point{1.0f, 2.5f, 0.0f});
tb.AddStructArray("vertices", std::span<const Point>(pts));
```

### Вкладені таблиці

```cpp
TableBuilder address;
address.Add<int32_t>("zip", 49000).AddString("city", "Дніпро");

TableBuilder person;
person.AddString("name", "Іван").AddTable("address", std::move(address));
```

---

## Приклади використання

### Базовий цикл: запис → читання

```cpp
#include "TableBuilder.hpp"
#include "View.hpp"

TableBuilder tb;
tb.Add<int32_t>("age", 25)
  .AddString("name", "Олена")
  .Add<double>("score", 98.5);

Buffer buf = tb.Finish();
View view(buf);

// доступ за індексом
int32_t age  = view.ReadTable<int32_t>(0);
auto    name = view.ReadTableString(1);

// доступ за іменем
int32_t age2  = view.ReadTable<int32_t>("age");
auto    name2 = view.ReadTableString("name");

// синтаксис через проксі
int32_t age3  = view["age"].as<int32_t>();
auto    name3 = view["name"].asString();
```

### HashedView — O(1) повторний доступ

```cpp
#include "HashedView.hpp"

HashedView hv(buf);  // будує unordered_map один раз

float temp     = hv.ReadTable<float>("temp");
float humidity = hv.ReadTable<float>("humidity");
// обидва звернення — O(1) замість O(n) лінійного сканування
```

### MutableView — in-place зміна поля

```cpp
#include "MutableView.hpp"

MutableView mv(buf);           // buf має бути owned (не borrowed/mmap)

mv.Set<int32_t>("status", 2);  // пише безпосередньо в буфер
mv.Set<float>  ("temp",   37.2f);

int32_t s = mv.Get<int32_t>("status");  // 2
auto    d = mv.GetString("device");     // string_view, без копії
```

### Збереження та читання файлу

```cpp
// запис
buf.save_to_file("data.rvb");

// читання з копіюванням у пам'ять
Buffer loaded = Buffer::from_file("data.rvb");

// zero-copy через mmap (OS завантажує лише потрібні сторінки)
Buffer mapped = Buffer::mmap_file("data.rvb");
View view(mapped);
```

### Вибіркове читання запису з масиву (O(1))

```cpp
struct WeatherRecord {
    int32_t timestamp, station_id;
    float   temperature, humidity, pressure, wind_speed;
};

Buffer mapped = Buffer::mmap_file("weather_1m.rvb");
View   view(mapped);
auto   span = view.ReadTableStructArray<WeatherRecord>("records");

// прямий доступ за індексом — offset = index * sizeof(WeatherRecord)
const WeatherRecord& r = span[499'999];
```

### Вкладені таблиці

```cpp
TableBuilder addr;
addr.Add<int32_t>("zip", 49000).AddString("city", "Дніпро");

TableBuilder person;
person.AddString("name", "Іван").AddTable("address", std::move(addr));

Buffer buf = person.Finish();
View view(buf);

View addr_view = view["address"].asTable();
int32_t zip  = addr_view.ReadTable<int32_t>("zip");
auto    city = addr_view.ReadTableString("city");
```

### Ітерація по полях

```cpp
for (auto [key, proxy] : view)
    std::cout << key << "\n";
```

### Верифікація буфера

```cpp
#include "Verifier.hpp"

Verifier v(buf);
if (!v.VerifyBuffer())
    throw std::runtime_error("пошкоджений буфер");
```

### Zero-copy зріз буфера

```cpp
// borrow — вказівник без копіювання, без ownership
Buffer borrowed = Buffer::borrow(ptr, size);

// slice — підбуфер без копіювання
Buffer head = buf.slice(0, 8);
```

---

## Протоколи та мережа

`Serializer<P>` і `Deserializer<P>` надають єдиний API для двох протоколів:

```cpp
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "Protocol/BinaryProtocol.hpp"
#include "Protocol/JsonProtocol.hpp"

// бінарний формат
Serializer<BinaryProtocol> s;
s.AddString("name", "Іван").Add<int32_t>("age", 25);
Buffer bin = s.Finish();

// JSON формат — той самий код, інший шаблонний параметр
Serializer<JsonProtocol> sj;
sj.AddString("name", "Іван").Add<int32_t>("age", 25);
Buffer json = sj.Finish();  // → {"name":"Іван","age":25}

// десеріалізація
Deserializer<BinaryProtocol> d(bin);
std::cout << d["name"].asString() << "\n";
std::cout << d["age"].as<int32_t>() << "\n";
```

### Передача через TCP

```cpp
// клієнт
Serializer<BinaryProtocol> s;
s.AddString("name", "Іван").Add<int32_t>("age", 25);
auto conn = SocketTransport::connect("127.0.0.1", 9000);
conn.send(s.Finish());

// сервер
auto listener = SocketTransport::listen(9000);
auto conn     = listener.accept();
Buffer buf    = conn.recv();
Deserializer<BinaryProtocol> d(buf);
std::cout << d["name"].asString() << "\n";
```

---

## Управління буфером

`Buffer` підтримує чотири режими зберігання:

| Режим | Як отримати | Ownership | Мутабельність |
|---|---|---|---|
| Owned (`std::vector`) | `TableBuilder::Finish()`, `Buffer::from_file()` | так | так |
| Controlled ptr | `Buffer(ptr, size, deleter)` | так (custom deleter) | так |
| Borrowed | `Buffer::borrow(ptr, size)` | ні | ні |
| mmap | `Buffer::mmap_file(path)` | так (OS unmap при деструкції) | ні |

```cpp
// власний буфер із custom deleter
Buffer buf(malloc_ptr, size, [](uint8_t* p){ std::free(p); });

// клонування
Buffer copy = buf.clone();
```

---

## Результати бенчмарків

Виміри виконано на Windows 11, Debug build. Порівняння з `yyjson` (JSON) та `reflect-cpp` (JSON з рефлексією).

### Серіалізація (3 поля: int32, string, double)

| | RawView | JsonWriter | reflect-cpp JSON |
|---|---|---|---|
| Час | ~120 нс | ~350 нс | ~800 нс |
| Розмір буфера | ~60 байт | ~45 байт | ~45 байт |

### Доступ до поля за іменем

| | RawView `View` | RawView `HashedView` | JsonReader | reflect-cpp |
|---|---|---|---|---|
| Час | ~15 нс | ~10 нс | ~80 нс | ~600 нс |

### Вибіркове читання (1 000 000 записів, запис №500 000)

| | RawView mmap | CSV лінійний скан |
|---|---|---|
| Час доступу | 260 мкс | 967 мс |
| Реальний I/O | ~4 КБ | ~19 МБ |
| Прискорення | **~3700×** | — |

### Завантаження датасету (100 000 записів)

| | RawView mmap | CSV парсинг |
|---|---|---|
| Час завантаження | 5 820 мкс | 505 мс |
| Розмір файлу | 2 343 КБ (−40%) | 3 842 КБ |
| Прискорення | **~86×** | — |

---

## Структура проекту

```
rawView/
├── include/
│   ├── Buffer.hpp           — управління пам'яттю (owned/borrowed/mmap)
│   ├── Builder.hpp          — низькорівневий будівник байтів
│   ├── TableBuilder.hpp     — будівник іменованих полів
│   ├── View.hpp             — read-only доступ до буфера
│   ├── HashedView.hpp       — O(1) доступ за іменем
│   ├── MutableView.hpp      — in-place мутація полів
│   ├── FieldProxy.hpp       — синтаксис view["key"].as<T>()
│   ├── StringArrayView.hpp  — zero-copy ітерація масиву рядків
│   ├── Verifier.hpp         — перевірка цілісності буфера
│   ├── Serializer.hpp       — псевдонім Protocol::Writer
│   ├── Deserializer.hpp     — псевдонім Protocol::Reader
│   └── Protocol/
│       ├── BinaryProtocol.hpp
│       ├── BinaryReader.hpp
│       ├── JsonProtocol.hpp
│       ├── JsonWriter.hpp
│       ├── JsonReader.hpp
│       └── JsonFieldProxy.hpp
│   └── Transport/
│       └── SocketTransport.hpp
├── src/
│   ├── main.cpp             — демонстраційний запуск
│   ├── server.cpp           — TCP-сервер (приклад)
│   └── client.cpp           — TCP-клієнт (приклад)
├── examples/
│   ├── iot_telemetry.cpp    — серіалізація, mmap, HashedView, MutableView
│   ├── selective_read.cpp   — вибіркове читання з 1M записів
│   └── csv_to_rvb.cpp       — конвертація CSV + агрегація
├── utests/                  — unit-тести (GoogleTest)
├── benchmarks/
│   └── bench.cpp            — бенчмарки (Google Benchmark)
└── CMakeLists.txt
```
