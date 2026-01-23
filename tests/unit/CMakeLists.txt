# Unit tests for Yuki-Frame

# Source files needed for tests (excluding main.c)
set(FRAMEWORK_LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/logger.c
    ${CMAKE_SOURCE_DIR}/src/core/event.c
    ${CMAKE_SOURCE_DIR}/src/core/tool.c
    ${CMAKE_SOURCE_DIR}/src/core/config.c
    ${CMAKE_SOURCE_DIR}/src/core/control.c
    ${CMAKE_SOURCE_DIR}/src/core/control_api.c
    ${CMAKE_SOURCE_DIR}/src/core/debug.c
)

# Platform-specific sources
if(WIN32)
    list(APPEND FRAMEWORK_LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/platform/platform_windows.c)
else()
    list(APPEND FRAMEWORK_LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/platform/platform_linux.c)
endif()

# Test: Event module
add_executable(test_event test_event.c ${FRAMEWORK_LIB_SOURCES})
target_include_directories(test_event PRIVATE ${CMAKE_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(test_event PRIVATE ws2_32)
else()
    target_link_libraries(test_event PRIVATE pthread rt)
endif()
add_test(NAME event_tests COMMAND test_event)

# Test: Config module
add_executable(test_config test_config.c ${FRAMEWORK_LIB_SOURCES})
target_include_directories(test_config PRIVATE ${CMAKE_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(test_config PRIVATE ws2_32)
else()
    target_link_libraries(test_config PRIVATE pthread rt)
endif()
add_test(NAME config_tests COMMAND test_config)

# Test: Tool module
add_executable(test_tool test_tool.c ${FRAMEWORK_LIB_SOURCES})
target_include_directories(test_tool PRIVATE ${CMAKE_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(test_tool PRIVATE ws2_32)
else()
    target_link_libraries(test_tool PRIVATE pthread rt)
endif()
add_test(NAME tool_tests COMMAND test_tool)

# Custom target to run all unit tests
add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_event test_config test_tool
    COMMENT "Running unit tests..."
)

message(STATUS "Unit tests configured")
