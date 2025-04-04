# Find the Qt6 modules
find_package(Qt6 REQUIRED COMPONENTS Core Network Location)

# Add the network test executable
add_executable(testOgnNet
    testOgnNet.cpp
    TrafficDataSource_Ogn.cpp
    ../../src/traffic/TrafficDataSource_OgnParser.cpp
    ../../src/traffic/TrafficFactorAircraftType.cpp
)

# Include the source directory to find headers
target_include_directories(testOgnNet
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/units
)

# Link the test executable with the necessary libraries
target_link_libraries(testOgnNet
    PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Location
)

# Add compile definitions specific to this target
target_compile_definitions(testOgnNet
    PRIVATE
    NETWORK_TEST
)
