#
# Demo to flash the RPI3
#
# Copyright (C) 2019-2020, Hensoldt Cyber GmbH
#

cmake_minimum_required(VERSION 3.7.2)

os_set_config_file("system_config.h" CREATE_CONFIG_PROJECT)


#-------------------------------------------------------------------------------
project(rpi3_flasher C)

set(LibUtilsDefaultZfLogLevel 5 CACHE STRING "" FORCE)

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

RamDisk_DeclareCAmkESComponent(
    RamDisk
    system_config
    IMAGE
        flash.c
)

TimeServer_DeclareCAmkESComponent(
    TimeServer
)

DeclareCAmkESComponent_RPI_SPI_Flash(
    Flash
    LIBS
        TimeServer_client
)

DeclareAndCreateCamkESSystem(demo.camkes)
GenerateSimulateScript()
