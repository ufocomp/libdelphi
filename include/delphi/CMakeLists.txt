cmake_minimum_required(VERSION 3.6.0)

file(GLOB hpp_files *.hpp)

install(FILES ${hpp_files} DESTINATION include/delphi)
