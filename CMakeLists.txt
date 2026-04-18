cmake_minimum_required(VERSION 3.20)
project(rawView)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE include
)

find_package(GTest REQUIRED)
enable_testing()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "utests/*.cpp")

add_executable(UTests
    ${TEST_SOURCES}
)

target_include_directories(UTests PRIVATE include)

target_link_libraries(UTests
    PRIVATE GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(UTests)