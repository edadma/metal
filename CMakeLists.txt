cmake_minimum_required(VERSION 3.13)

# Options
option(DEBUG_OPTION "Enable debug output" ON)

# Check TARGET_PLATFORM variable
if (NOT DEFINED TARGET_PLATFORM)
    set(TARGET_PLATFORM "host")
endif ()

if (TARGET_PLATFORM STREQUAL "pico")
    set(BUILD_FOR_PICO ON)
    message(STATUS "Building for Pico")
elseif (TARGET_PLATFORM STREQUAL "host")
    set(BUILD_FOR_PICO OFF)
    message(STATUS "Building for host (nix)")
else ()
    message(FATAL_ERROR "Unknown TARGET_PLATFORM: ${TARGET_PLATFORM}. Use 'host' or 'pico'")
endif ()

if (BUILD_FOR_PICO)
    # Pico setup - MUST include pico_sdk_import.cmake BEFORE project()
    set(PICO_BOARD pico_w)
    include(pico_sdk_import.cmake)
endif ()

# Project declaration
if (BUILD_FOR_PICO)
    project(metal C CXX ASM)
    set(CMAKE_C_STANDARD 11)
else ()
    project(metal C)
    set(CMAKE_C_STANDARD 23)
endif ()

# Initialize Pico SDK after project declaration
if (BUILD_FOR_PICO)
    pico_sdk_init()
endif ()

# Common source files
set(METAL_SOURCES
        src/main.c
        src/debug.c
)

# Common include directories
set(METAL_INCLUDES
        include
)

if (BUILD_FOR_PICO)
    # Pico executable
    add_executable(metal ${METAL_SOURCES})

    target_include_directories(metal PRIVATE ${METAL_INCLUDES})

    # Pico-specific settings
    pico_enable_stdio_usb(metal 1)
    pico_enable_stdio_uart(metal 0)

    target_link_libraries(metal
            pico_stdlib
            hardware_gpio
            hardware_rtc
    )

    pico_add_extra_outputs(metal)

    target_compile_definitions(metal PRIVATE METAL_TARGET_PICO=1)

else ()
    # Host executable
    add_executable(metal ${METAL_SOURCES})

    target_include_directories(metal PRIVATE ${METAL_INCLUDES})

    # Host-specific settings
    target_compile_options(metal PRIVATE -Wall -Wextra)

    # Link pthread on host
    target_link_libraries(metal pthread)
endif ()

# Debug option
if (DEBUG_OPTION)
    target_compile_definitions(metal PRIVATE DEBUG_ENABLED=1)
endif ()
