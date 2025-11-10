cmake_minimum_required(VERSION 3.10)

add_subdirectory(delphi)

if (BUILD_STATIC_LIB OR BUILD_SHARED_LIB)
    install(FILES delphi.hpp DESTINATION include)
endif()
