cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(PICO_SDK_PATH "C:\\pico-sdk")

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(main C CXX ASM)

pico_sdk_init()

add_executable(main main.c ssd1306/ssd1306.c ssd1306/ssd1306_fonts.c joystick/JoystickPi.c menu/menu.c I2C/i2c.c sfp_8472/a0h.c  sfp_8472/a2h.c sfp_8472/sfp.c)

pico_set_program_name(main "main.c")
pico_set_program_version(main "0.1")

pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main  1)

target_link_libraries(main pico_stdlib)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(main
		      hardware_i2c
		      hardware_adc
		      hardware_pwm)

pico_add_extra_outputs(main)
