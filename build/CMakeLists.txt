# cmake build list
# Copyright (C) 2013 toney
# @author: toney
# 2014-05-4
# network cmake

cmake_minimum_required(VERSION 2.8.3)


include(macros.cmake)
include(config.cmake)


clear_all_list()

add_include_path(../src)
add_include_path(../public)

add_source_files(../src/*.*)
build_static_lib(network ../)

