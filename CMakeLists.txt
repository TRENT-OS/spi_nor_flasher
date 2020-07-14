#
# Demo to flash the RPI3
#
# Copyright (C) 2019-2020, Hensoldt Cyber GmbH
#

cmake_minimum_required(VERSION 3.7.2)

os_set_config_file(system_config.h)

#-------------------------------------------------------------------------------
project(system_config C)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
    )

#-------------------------------------------------------------------------------
project(demo_flash_rpi3 C)

DeclareCAmkESComponent(
    FlashRpi3
    SOURCES
        components/FlashRpi3/src/FlashRpi3.c
    C_FLAGS
        -Wall
        -Werror
    LIBS
        system_config
        os_core_api
        os_libs
)

DeclareCAmkESComponent_RamDisk(
    RamDisk
    system_config
    IMAGE
        flash.c
)

DeclareCAmkESComponent_SPI(
    SPI
)

DeclareCAmkESComponent_TimeServer(
    TimeServer
)

DeclareCAmkESComponent_Storage_Flash_SPI(
    Flash
    LIBS
        TimeServer_client
)

DeclareAndCreateCamkESSystem(demo.camkes)
GenerateSimulateScript()
