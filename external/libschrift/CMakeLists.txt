cmake_minimum_required(VERSION 3.0)

project(libschrift)
add_library(schrift "schrift.c")
target_include_directories(schrift PRIVATE "schrift.h")