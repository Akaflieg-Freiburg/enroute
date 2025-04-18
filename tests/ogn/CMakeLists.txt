# Find the Qt6Test, Qt6Network, Qt6Location, and Qt6Bluetooth modules
find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat Test Network Location Positioning)

# Add the test executable
add_executable(unittestOgn
    TrafficDataSource_OgnTest.cpp
    ../../src/traffic/TrafficDataSource_OgnParser.cpp
    ../../src/traffic/TrafficFactorAircraftType.cpp
)

# Include the source directory to find headers
target_include_directories(unittestOgn
    PRIVATE
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/units
)

# Link the test executable with the necessary libraries
target_link_libraries(unittestOgn
    PRIVATE
    Qt6::Core
    Qt6::Core5Compat
    Qt6::Test
    Qt6::Network
    Qt6::Location
    Qt6::Positioning
)

# Add compile definitions specific to this target
target_compile_definitions(unittestOgn
    PRIVATE
    QT_DISABLE_DEPRECATED_BEFORE=0x051200
)

# Register the test executable with CTest
add_test(NAME TrafficDataSource_OgnTest COMMAND unittestOgn)

