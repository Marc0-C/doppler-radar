cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(blink
    blink.c
)

# ---- CMSIS include paths ----
target_include_directories(blink PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../CMSISDSP/CMSIS-DSP/Include
    ${CMAKE_CURRENT_LIST_DIR}/../CMSISDSP/CMSIS_6/CMSIS/Core/Include
)

target_link_libraries(blink pico_stdlib hardware_adc ${CMAKE_CURRENT_LIST_DIR}/../CMSISDSP/build/bin_dsp/libCMSISDSP.a)

pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 1)

pico_add_extra_outputs(blink)
