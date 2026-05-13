include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.1
)
FetchContent_MakeAvailable(benchmark)

set(REFLECTCPP_JSON ON)
set(REFLECTCPP_MSGPACK OFF)
FetchContent_Declare(
    reflectcpp
    GIT_REPOSITORY https://github.com/getml/reflect-cpp.git
    GIT_TAG v0.14.1
)
FetchContent_MakeAvailable(reflectcpp)

file(GLOB BENCH_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(Benchmarks ${BENCH_SOURCES})
target_include_directories(Benchmarks PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(Benchmarks PRIVATE benchmark::benchmark_main yyjson reflectcpp)

if(WIN32)
    target_compile_definitions(Benchmarks PRIVATE NOGDI NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
