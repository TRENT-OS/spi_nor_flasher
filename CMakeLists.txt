#
# Demo to flash the RPI3
#
# Copyright (C) 2019-2024, HENSOLDT Cyber GmbH
# 
# SPDX-License-Identifier: GPL-2.0-or-later
#
# For commercial licensing, contact: info.cyber@hensoldt.net
#


cmake_minimum_required(VERSION 3.7.2)

#-------------------------------------------------------------------------------
# use the SDK
find_package("os-sdk" REQUIRED)
os_sdk_set_defaults()
set(LibUtilsDefaultZfLogLevel 5 CACHE STRING "" FORCE)
os_sdk_setup(CONFIG_FILE "system_config.h" CONFIG_PROJECT "system_config")

#-------------------------------------------------------------------------------
project(rpi3_flasher C)

CAmkESAddCPPInclude("plat/${PLATFORM}")

DeclareCAmkESComponent(
    FlashRpi3
    SOURCES
        components/FlashRpi3/src/FlashRpi3.c
    C_FLAGS
        -Wall
        -Werror
    LIBS
        system_config
        lib_debug
        os_core_api
)

RamDisk_DeclareCAmkESComponent(
    RamDisk
    IMAGE
        flash.c
)

TimeServer_DeclareCAmkESComponent(
    TimeServer
)

RPi_SPI_Flash_DeclareCAmkESComponent(
    RPi_SPI_Flash
    LIBS
        TimeServer_client
)

os_sdk_create_CAmkES_system("demo.camkes")
