cmake_minimum_required(VERSION 3.6.0)
project(libdelphi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Settings
# ----------------------------------------------------------------------------------------------------------------------
set(BUILD_STATIC_LIB    OFF CACHE BOOL "Build a static library")
set(BUILD_SHARED_LIB    OFF CACHE BOOL "Build a shared library")

set(WITH_POSTGESQL      OFF CACHE BOOL "Build with PostgreSQL")
set(WITH_SQLITE3        OFF CACHE BOOL "Build with Sqlite3")

set(EXTRA_WARNING_MODE  OFF CACHE BOOL "Add extra warnings in debug mode")
# ----------------------------------------------------------------------------------------------------------------------

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND EXTRA_WARNING_MODE)
    include(contrib/cmake/enable_cxx_compiler_flag_if_supported.cmake)
    enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_DEBUG "-Wall")
    enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_DEBUG "-Wextra")
    enable_cxx_compiler_flag_if_supported(CMAKE_CXX_FLAGS_DEBUG "-pedantic")
endif()

add_compile_options("-DDELPHI_LIB_EXPORTS")

# -Iinclude
include_directories(include)
include_directories(src)

# add library directories
add_subdirectory(include)
add_subdirectory(src)

# Delphi classes for C++
# ----------------------------------------------------------------------------------------------------------------------
set(DELPHI_LIB_NAME delphi)

if (WITH_POSTGESQL)
    message(STATUS "Using PostgreSQL.")
    find_package(PostgreSQL REQUIRED)
    set(PQ_LIB_NAME "pq")
    add_compile_options("-DDELPHI_POSTGRESQL")
endif()

if (WITH_SQLITE3)
    message(STATUS "Using SQLite3.")
    set(SQLITE3_LIB_NAME "sqlite3")
    add_compile_options("-DDELPHI_SQLITE3")
endif()

if (BUILD_STATIC_LIB)
    # build the static library
    add_library(${DELPHI_LIB_NAME}_static STATIC $<TARGET_OBJECTS:delphi>)
    set_target_properties(${DELPHI_LIB_NAME}_static PROPERTIES OUTPUT_NAME "${DELPHI_LIB_NAME}")
    target_link_libraries(${DELPHI_LIB_NAME}_static pthread ${SQLITE3_LIB_NAME} ${PQ_LIB_NAME})
    install(TARGETS ${DELPHI_LIB_NAME}_static DESTINATION lib)
endif()

if (BUILD_SHARED_LIB)
    # build the static library
    add_library(${DELPHI_LIB_NAME}_shared SHARED $<TARGET_OBJECTS:delphi>)
    set_target_properties(${DELPHI_LIB_NAME}_shared PROPERTIES OUTPUT_NAME "${DELPHI_LIB_NAME}")
    target_link_libraries(${DELPHI_LIB_NAME}_shared pthread ${SQLITE3_LIB_NAME} ${PQ_LIB_NAME})
    install(TARGETS ${DELPHI_LIB_NAME}_shared DESTINATION lib)
endif()

# ----------------------------------------------------------------------------------------------------------------------

if (BUILD_STATIC_LIB OR BUILD_SHARED_LIB)
    # add pkg-config
    configure_file("contrib/${DELPHI_LIB_NAME}.pc.in" "${DELPHI_LIB_NAME}.pc" @ONLY)
    install(FILES "${CMAKE_BINARY_DIR}/${DELPHI_LIB_NAME}.pc" DESTINATION lib/pkgconfig)
endif()