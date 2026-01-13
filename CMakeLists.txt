cmake_minimum_required(VERSION 3.10)

project(yuki-frame 
    VERSION 2.0.0
    DESCRIPTION "Event-driven tool orchestration framework with integrated control and debug"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Detect platform
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS -D_CRT_SECURE_NO_WARNINGS)
    message(STATUS "Building for Windows")
else()
    add_definitions(-DPLATFORM_LINUX -D_POSIX_C_SOURCE=200809L)
    message(STATUS "Building for Linux/Unix")
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
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

# Source files
set(FRAMEWORK_SOURCES
    src/core/main.c
    src/core/logger.c
    src/core/event.c
    src/core/tool.c
    src/core/config.c
    src/core/control.c
    src/core/debug.c
)

# Platform-specific sources
if(WIN32)
    list(APPEND FRAMEWORK_SOURCES src/core/platform_windows.c)
else()
    list(APPEND FRAMEWORK_SOURCES src/core/platform_linux.c)
endif()

# Main executable
add_executable(yuki-frame ${FRAMEWORK_SOURCES})

# Platform-specific linking
if(WIN32)
    target_link_libraries(yuki-frame PRIVATE ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(yuki-frame PRIVATE Threads::Threads rt)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS yuki-frame
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
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
    GETTING_STARTED.md
    ARCHITECTURE.md
    TOOL_DEVELOPMENT.md
    CHANGELOG.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# Package generation
set(CPACK_PACKAGE_NAME "yuki-frame")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Event-driven tool orchestration framework")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)

message(STATUS "")
message(STATUS "Yuki-Frame v${PROJECT_VERSION} Configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
