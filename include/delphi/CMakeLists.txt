cmake_minimum_required(VERSION 3.6.0)

file(GLOB hpp_files *.hpp)

if (BUILD_STATIC_LIB OR BUILD_SHARED_LIB)
    install(FILES ${hpp_files} DESTINATION include/delphi)
endif()