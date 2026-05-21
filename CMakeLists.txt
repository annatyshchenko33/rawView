cmake_minimum_required(VERSION 3.20)
project(rawView)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)


FetchContent_Declare(
    yyjson
    GIT_REPOSITORY https://github.com/ibireme/yyjson.git
    GIT_TAG 0.12.0
)
FetchContent_MakeAvailable(yyjson)


set(SOURCES src/main.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE yyjson)


find_package(GTest REQUIRED)
enable_testing()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "utests/*.cpp")
add_executable(UTests ${TEST_SOURCES})
target_include_directories(UTests PRIVATE include)
target_link_libraries(UTests PRIVATE GTest::gtest_main yyjson)

include(GoogleTest)
gtest_discover_tests(UTests)

option(BUILD_BENCHMARKS "Build benchmarks" OFF)
if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

add_executable(server src/server.cpp)
target_include_directories(server PRIVATE include)
target_link_libraries(server PRIVATE yyjson)

add_executable(client src/client.cpp)
target_include_directories(client PRIVATE include)
target_link_libraries(client PRIVATE yyjson)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
    target_link_libraries(client PRIVATE ws2_32)
endif()

option(BUILD_EXAMPLES "Build examples" OFF)
if(BUILD_EXAMPLES)
    add_executable(iot_telemetry  examples/iot_telemetry.cpp)
    add_executable(selective_read examples/selective_read.cpp)
    add_executable(csv_to_rvb     examples/csv_to_rvb.cpp)

    foreach(ex iot_telemetry selective_read csv_to_rvb)
        target_include_directories(${ex} PRIVATE include)
        target_link_libraries(${ex} PRIVATE yyjson)
    endforeach()
endif()