# Test suite for Yuki-Frame

enable_testing()

# Unit tests
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)

# Custom target to run all tests
add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_event test_config test_tool
    COMMENT "Running all tests..."
)

message(STATUS "Test suite configured")
