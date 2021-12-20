#
# CMakeLists.txt
# Copyright (C) 2021 Marius Greuel
#

cmake_minimum_required(VERSION 3.12)
project(libftdi)

add_library(libftdi STATIC
    src/ftdi.cpp
    )

target_include_directories(libftdi
    PUBLIC
    include
    )

if(MSVC)
    target_compile_options(libftdi PRIVATE /wd4267) # warning C4267: 'function': conversion from 'xxx' to 'xxx', possible loss of data
endif()
