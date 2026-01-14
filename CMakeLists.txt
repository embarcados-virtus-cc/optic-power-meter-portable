cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(main C CXX ASM)

pico_sdk_init()

add_executable(main main.c sfp_8472.c)

pico_set_program_name(main "main.c")
pico_set_program_version(main "0.1")

pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

target_link_libraries(main pico_stdlib)

#Ativar o DEBUG, para desativar comente essa linha
target_compile_definitions(main PRIVATE DEBUG)


target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(main
		      hardware_i2c
		      )

pico_add_extra_outputs(main)
