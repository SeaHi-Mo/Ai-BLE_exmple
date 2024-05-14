cmake_minimum_required(VERSION 3.15)

include(proj.conf)

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/components/*.c ${CMAKE_CURRENT_SOURCE_DIR}/components/BLE/*.c)

sdk_add_include_directories(.)
sdk_add_include_directories(main systemConfig)
sdk_add_include_directories(components/BLE)

# 添加HomeAssistant 库
list(REMOVE_ITEM SRC_FILES "./main/main.c")

target_sources(app PRIVATE ${SRC_FILES} ${sources})

sdk_set_main_file(main/main.c)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})
