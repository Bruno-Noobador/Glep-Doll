cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(glep_firmware C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_subdirectory($ENV{PICO_EXTRAS_PATH}/src/rp2_common/pico_sleep pico_sleep)


# --- LittleFS ---
add_library(littlefs
    littlefs/lfs.c
    littlefs/lfs_util.c
)

target_include_directories(littlefs PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/littlefs
)

# --- Executable ---
add_executable(glep_firmware
    glep_firmware.c
    lfs_config.c
)

target_link_libraries(glep_firmware
    pico_stdlib
    hardware_flash
    hardware_sync
    hardware_pwm
    pico_sleep
    littlefs
)

pico_enable_stdio_usb(glep_firmware 1)
pico_enable_stdio_uart(glep_firmware 0)

pico_add_extra_outputs(glep_firmware)