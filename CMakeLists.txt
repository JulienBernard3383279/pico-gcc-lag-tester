cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_FETCH_FROM_GIT 1)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(pico_sdk_import.cmake)

project(lag_test_controller_melee CXX C)

pico_sdk_init()

add_executable(lag_test_controller_melee
    src/main.cpp
    src/usb/logic.cpp
    src/usb/protocol.cpp
    src/joybus.cpp
)

pico_generate_pio_header(lag_test_controller_melee ${CMAKE_CURRENT_LIST_DIR}/pio/my_pio.pio)

add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio
        COMMAND Pioasm ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
        )
        
target_include_directories(lag_test_controller_melee PRIVATE include)

pico_set_linker_script(lag_test_controller_melee ../lag_test_controller_melee.ld)

target_link_libraries(lag_test_controller_melee hardware_adc hardware_pio pico_util pico_stdlib pico_multicore hardware_pwm hardware_resets hardware_irq)

pico_add_extra_outputs(lag_test_controller_melee)
