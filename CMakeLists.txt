set(METAL_VERSION "0.0.1")

cmake_minimum_required(VERSION 3.13)

# Debug option
option(DEBUG_OPTION "Enable debug output" ON)
option(COPY_EXECUTABLES_TO_ROOT "Copy built executables to repository root" ON)

# Platform selection
if (NOT DEFINED TARGET_PLATFORM)
    if (WIN32)
        set(TARGET_PLATFORM "windows")
    else ()
        set(TARGET_PLATFORM "host")
    endif ()
endif ()

# Pico setup (before project)
if (TARGET_PLATFORM STREQUAL "pico")
    set(PICO_BOARD pico_w)
    include(pico_sdk_import.cmake)
endif ()

if (TARGET_PLATFORM STREQUAL "windows")
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")
endif ()

# Project
if (TARGET_PLATFORM STREQUAL "pico")
    project(metal C CXX ASM)
else ()
    project(metal C)
endif ()

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
elseif (TARGET_PLATFORM STREQUAL "windows")
    file(GLOB PLATFORM_SOURCES "platform/windows/src/*.c")
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

    # Copy UF2 to repository root
    if (COPY_EXECUTABLES_TO_ROOT)
        add_custom_command(TARGET metal POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_BINARY_DIR}/metal.uf2
                ${CMAKE_SOURCE_DIR}/metal-v${METAL_VERSION}-pico.uf2
                COMMENT "Copying UF2 file to repository root"
        )
    endif ()
elseif (TARGET_PLATFORM STREQUAL "host")
    target_compile_definitions(metal PRIVATE TARGET_HOST=1)
    target_link_libraries(metal pthread)

    # Copy Linux executable to repository root
    if (COPY_EXECUTABLES_TO_ROOT)
        add_custom_command(TARGET metal POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:metal>
                ${CMAKE_SOURCE_DIR}/metal-v${METAL_VERSION}-linux
                COMMENT "Copying Linux executable to repository root"
        )
    endif ()
elseif (TARGET_PLATFORM STREQUAL "windows")
    target_compile_definitions(metal PRIVATE TARGET_WINDOWS=1)

    # Copy Windows executable to repository root
    if (COPY_EXECUTABLES_TO_ROOT)
        add_custom_command(TARGET metal POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:metal>
                ${CMAKE_SOURCE_DIR}/metal-v${METAL_VERSION}-windows.exe
                COMMENT "Copying Windows executable to repository root"
        )
    endif ()
endif ()

if (DEBUG_OPTION)
    target_compile_definitions(metal PRIVATE DEBUG_ENABLED=1)
endif ()

# Windows-specific compiler settings
if (TARGET_PLATFORM STREQUAL "windows")
    if (MINGW)
        # Static linking to avoid DLL dependencies when cross-compiling
        set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")
    endif ()
endif ()

target_compile_definitions(metal PRIVATE
        METAL_VERSION="${METAL_VERSION}"
)

message(STATUS "Metal version: ${METAL_VERSION}")
message(STATUS "Building Metal for platform: ${TARGET_PLATFORM}")
message(STATUS "Copy executables to root: ${COPY_EXECUTABLES_TO_ROOT}")
message(STATUS "Debug option: ${DEBUG_OPTION}")
