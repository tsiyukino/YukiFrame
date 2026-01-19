# Integration tests for Yuki-Frame

# Python-based integration tests
find_package(Python3 COMPONENTS Interpreter)

if(Python3_FOUND)
    add_test(
        NAME integration_tests
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_integration.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    message(STATUS "Integration tests configured (Python ${Python3_VERSION})")
else()
    message(WARNING "Python3 not found - integration tests will not be available")
endif()
