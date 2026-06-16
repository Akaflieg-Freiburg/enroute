# Standalone unit-test project for enroute's pure-logic C++.
#
# The app itself is a monolithic executable, so tests recompile just the
# sources under test plus Qt6::Test. Build separately, e.g.:
#
#   cmake -S tests -B ../build-tests
#   cmake --build ../build-tests
#   ctest --test-dir ../build-tests --output-on-failure
#
cmake_minimum_required(VERSION 3.21)
project(enroute_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.9 COMPONENTS Core Network Qml Test REQUIRED)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Units are small and interdependent; compile them all.
file(GLOB UNITS_SOURCES ${SRC_DIR}/units/*.cpp)

enable_testing()

qt_add_executable(tst_windfieldprovider
    tst_windfieldprovider.cpp
    ${SRC_DIR}/weather/WindFieldProvider.cpp
    ${SRC_DIR}/weather/ForecastMapProvider.cpp
    ${SRC_DIR}/weather/Wind.cpp
    ${UNITS_SOURCES}
)
target_include_directories(tst_windfieldprovider PRIVATE ${SRC_DIR})
target_link_libraries(tst_windfieldprovider PRIVATE
    Qt6::Core Qt6::Network Qt6::Qml Qt6::Test)
add_test(NAME windfieldprovider COMMAND tst_windfieldprovider)
