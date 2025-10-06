# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(mqtt_with_psk C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(mqtt_with_psk 
    main.c
    src/wifi.c
    src/mqtt.c
    src/shared_vars.c
    src/pico_net.c
    src/temperature.c
    src/ssd1306.c
    src/botoes.c
)

pico_set_program_name(mqtt_with_psk "mqtt_with_psk")
pico_set_program_version(mqtt_with_psk "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(mqtt_with_psk 0)
pico_enable_stdio_usb(mqtt_with_psk 1)

# Add the standard library to the build
target_link_libraries(mqtt_with_psk
    hardware_adc
    hardware_i2c
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    
    # Criptografia
    pico_mbedtls
    pico_lwip_mbedtls

    # Opcional: Utilitário para entrar no modo bootloader com clique duplo
    pico_bootsel_via_double_reset
)

# Add the standard include files to the build
target_include_directories(mqtt_with_psk PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Configuração do mbedTLS
target_compile_definitions(mqtt_with_psk PRIVATE MBEDTLS_USER_CONFIG_FILE="inc/mbedtls_config.h")

pico_add_extra_outputs(mqtt_with_psk)

