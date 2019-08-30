cmake_minimum_required(VERSION 3.14)

file(GLOB cpp_files *.cpp)

add_library(delphi OBJECT ${cpp_files})

set_property(TARGET delphi PROPERTY POSITION_INDEPENDENT_CODE ON)
