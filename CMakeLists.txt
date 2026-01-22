cmake_minimum_required(VERSION 3.10)

project(yuki-frame 
    VERSION 2.0.0
    DESCRIPTION "Event-driven tool orchestration framework"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Platform detection
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS -D_CRT_SECURE_NO_WARNINGS)
    message(STATUS "Building for Windows")
else()
    add_definitions(-DPLATFORM_LINUX -D_POSIX_C_SOURCE=200809L)
    message(STATUS "Building for Linux/Unix")
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -DDEBUG)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Core source files (shared between executables)
set(FRAMEWORK_LIB_SOURCES
    src/core/logger.c
    src/core/event.c
    src/core/tool.c
    src/core/config.c
    src/core/control.c
    src/core/debug.c
)

# Platform-specific sources
if(WIN32)
    list(APPEND FRAMEWORK_LIB_SOURCES src/platform/platform_windows.c)
else()
    list(APPEND FRAMEWORK_LIB_SOURCES src/platform/platform_linux.c)
endif()

# Main executable
add_executable(yuki-frame 
    src/core/main.c
    ${FRAMEWORK_LIB_SOURCES}
)

# Platform-specific linking for main executable
if(WIN32)
    target_link_libraries(yuki-frame PRIVATE ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(yuki-frame PRIVATE Threads::Threads rt)
endif()

# Control utility executable
add_executable(yuki-control 
    src/core/cli_control.c
    ${FRAMEWORK_LIB_SOURCES}
)

# Platform-specific linking for control utility
if(WIN32)
    target_link_libraries(yuki-control PRIVATE ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(yuki-control PRIVATE Threads::Threads rt)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS yuki-frame yuki-control
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/yuki_frame
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

if(WIN32)
    set(CONFIG_DIR "C:/ProgramData/yuki-frame")
    set(LOG_DIR "C:/ProgramData/yuki-frame/logs")
else()
    set(CONFIG_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/yuki-frame")
    set(LOG_DIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/log/yuki-frame")
endif()

install(DIRECTORY DESTINATION ${CONFIG_DIR})
install(DIRECTORY DESTINATION ${LOG_DIR})

install(FILES yuki-frame.conf.example
    DESTINATION ${CONFIG_DIR}
    RENAME yuki-frame.conf.example
)

install(FILES
    README.md
    TOOL_DEVELOPMENT.md
    DEVELOPMENT.md
    CHANGELOG.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# Testing
enable_testing()
add_subdirectory(tests)

# Package generation
set(CPACK_PACKAGE_NAME "yuki-frame")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Event-driven tool orchestration framework")
set(CPACK_PACKAGE_VENDOR "Yuki-Frame Project")
set(CPACK_PACKAGE_CONTACT "maintainer@yuki-frame.example")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)

# Build summary
message(STATUS "")
message(STATUS "========================================")
message(STATUS "Yuki-Frame v${PROJECT_VERSION} Build Configuration")
message(STATUS "========================================")
message(STATUS "  Platform:       ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler:     ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C Standard:     ${CMAKE_C_STANDARD}")
message(STATUS "========================================")
message(STATUS "  Executables:")
message(STATUS "    - yuki-frame (main framework)")
message(STATUS "    - yuki-control (control utility)")
message(STATUS "========================================")
message(STATUS "")
