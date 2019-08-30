cmake_minimum_required(VERSION 3.14)

file(GLOB hpp_files *.hpp)

install(FILES ${hpp_files} DESTINATION include/delphi)
