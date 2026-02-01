cmake_minimum_required(VERSION 3.10)

project(yuki-frame 
    VERSION 2.0.0
    DESCRIPTION "Event-driven tool orchestration framework for Windows"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Windows-only project
if(NOT WIN32)
    message(FATAL_ERROR "This project is Windows-only. Please build on Windows.")
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Windows definitions
add_definitions(-DPLATFORM_WINDOWS -D_CRT_SECURE_NO_WARNINGS)
message(STATUS "Building for Windows")

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX-)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Zi /Od /DDEBUG)
    else()
        add_compile_options(/O2 /DNDEBUG)
    endif()
else()
    message(WARNING "Non-MSVC compiler detected. This project is designed for MSVC.")
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Core source files
set(FRAMEWORK_LIB_SOURCES
    src/core/logger.c
    src/core/event.c
    src/core/tool.c
    src/core/tool_queue.c
    src/core/config.c
    src/core/control.c
    src/core/debug.c
    src/core/control_api.c
    src/core/control_socket.c
    src/platform/platform_windows.c
)

# Main executable
add_executable(yuki-frame 
    src/core/main.c
    ${FRAMEWORK_LIB_SOURCES}
)

# Windows-specific linking
target_link_libraries(yuki-frame PRIVATE ws2_32)

# Installation
include(GNUInstallDirs)

install(TARGETS yuki-frame
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/yuki_frame
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Windows paths
set(CONFIG_DIR "C:/ProgramData/yuki-frame")
set(LOG_DIR "C:/ProgramData/yuki-frame/logs")
set(DOC_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/yuki-frame")

install(DIRECTORY DESTINATION ${CONFIG_DIR})
install(DIRECTORY DESTINATION ${LOG_DIR})

install(FILES yuki-frame.conf.example
    DESTINATION ${CONFIG_DIR}
    RENAME yuki-frame.conf.example
)

# Install documentation
install(FILES README.md LICENSE
    DESTINATION ${DOC_DIR}
)

install(DIRECTORY docs/
    DESTINATION ${DOC_DIR}
    FILES_MATCHING PATTERN "*.md"
)

# Install console client
install(FILES yuki-console.py
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY tools/
    DESTINATION ${DOC_DIR}/examples/tools
    FILES_MATCHING PATTERN "*.py"
)

install(DIRECTORY examples/
    DESTINATION ${DOC_DIR}/examples
    FILES_MATCHING PATTERN "*"
)

# Testing
enable_testing()
add_subdirectory(tests)

# Package generation
set(CPACK_PACKAGE_NAME "yuki-frame")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Event-driven tool orchestration framework for Windows")
set(CPACK_PACKAGE_VENDOR "Yuki-Frame Project")
set(CPACK_PACKAGE_CONTACT "maintainer@yuki-frame.example")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_GENERATOR "ZIP;NSIS")

include(CPack)

# Build summary
message(STATUS "")
message(STATUS "========================================")
message(STATUS "Yuki-Frame v${PROJECT_VERSION} Build Configuration")
message(STATUS "========================================")
message(STATUS "  Platform:       Windows")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler:     ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C Standard:     ${CMAKE_C_STANDARD}")
message(STATUS "========================================")
message(STATUS "  INTEGRATED ARCHITECTURE:")
message(STATUS "  • Main executable:   yuki-frame.exe")
message(STATUS "  • Control API:       Integrated")
message(STATUS "  • Control Socket:    Integrated")
message(STATUS "  • Console Client:    yuki-console.py")
message(STATUS "========================================")
message(STATUS "")
