cmake_minimum_required(VERSION 3.28)

set(PICO_PLATFORM rp2350)
set(PICO_BOARD pico2)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

include(pico_sdk_import.cmake)

pico_sdk_init()

project(redip_cia C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(redip_cia
    src/main.c
    src/cia.c
    src/timer.c
    src/tod.c
    src/serial.c
    src/hardware.c
)

target_include_directories(redip_cia PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src/pio
)

pico_enable_stdio_usb(redip_cia 1)

target_compile_definitions(redip_cia PRIVATE PICO_RP2040_RP2350_A2_HFRCK_CONFIG=250000000)

pico_generate_pio_header(redip_cia ${CMAKE_CURRENT_LIST_DIR}/src/pio/bus_capture.pio)
pico_generate_pio_header(redip_cia ${CMAKE_CURRENT_LIST_DIR}/src/pio/bus_output.pio)
pico_generate_pio_header(redip_cia ${CMAKE_CURRENT_LIST_DIR}/src/pio/ports.pio)
pico_generate_pio_header(redip_cia ${CMAKE_CURRENT_LIST_DIR}/src/pio/timer_pio.pio)

target_link_libraries(redip_cia
    pico_stdlib
    hardware_pio
    hardware_dma
)

pico_add_extra_outputs(redip_cia)