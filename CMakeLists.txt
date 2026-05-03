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