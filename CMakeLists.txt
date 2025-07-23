cmake_minimum_required(VERSION 3.13)

# Platform selection
if (NOT DEFINED TARGET_PLATFORM)
    set(TARGET_PLATFORM "host")
endif ()

# Pico setup (before project)
if (TARGET_PLATFORM STREQUAL "pico")
    set(PICO_BOARD pico_w)
    include(pico_sdk_import.cmake)
endif ()

# Project
project(metal C)
set(CMAKE_C_STANDARD 23)

# Initialize Pico SDK
if (TARGET_PLATFORM STREQUAL "pico")
    pico_sdk_init()
endif ()

# Core Metal sources (always included)
file(GLOB CORE_SOURCES "src/*.c")

# Platform-specific sources
if (TARGET_PLATFORM STREQUAL "pico")
    file(GLOB PLATFORM_SOURCES "platform/pico/src/*.c")
elseif (TARGET_PLATFORM STREQUAL "host")
    file(GLOB PLATFORM_SOURCES "platform/host/src/*.c")
endif ()

# Create executable
add_executable(metal ${CORE_SOURCES} ${PLATFORM_SOURCES})

# Include directories
target_include_directories(metal PRIVATE
        include
        platform/${TARGET_PLATFORM}/include
)

# Platform-specific settings
if (TARGET_PLATFORM STREQUAL "pico")
    target_compile_definitions(metal PRIVATE TARGET_PICO=1)
    target_link_libraries(metal pico_stdlib hardware_gpio hardware_rtc)
    pico_enable_stdio_usb(metal 1)
    pico_enable_stdio_uart(metal 0)
    pico_add_extra_outputs(metal)
else ()
    target_compile_definitions(metal PRIVATE TARGET_HOST=1)
    target_link_libraries(metal pthread)
endif ()

# Debug option
option(DEBUG_OPTION "Enable debug output" ON)

if (DEBUG_OPTION)
    target_compile_definitions(metal PRIVATE DEBUG_ENABLED=1)
endif ()
