set(CMAKE_VERSION_MINIMUM 3.31)
cmake_minimum_required(VERSION ${CMAKE_VERSION_MINIMUM})
message(STATUS "Cmake version: ${CMAKE_VERSION} (>= ${CMAKE_VERSION_MINIMUM} ✓)")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(mcpplibs-primitives VERSION 0.4.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_GCC_VERSION_MINIMUM 14.0)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${COMPILER_GCC_VERSION_MINIMUM})
        message(FATAL_ERROR "GCC version too old for C++ Modules.\nGCC version: ${CMAKE_CXX_COMPILER_VERSION} (>= ${COMPILER_GCC_VERSION_MINIMUM} X)")
    else()
        message(STATUS "GCC version: ${CMAKE_CXX_COMPILER_VERSION} (>= ${COMPILER_GCC_VERSION_MINIMUM} ✓)")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_CLANG_VERSION_MINIMUM 17.0)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${COMPILER_CLANG_VERSION_MINIMUM})
        message(FATAL_ERROR "Clang version too old for C++ Modules.\nClang version: ${CMAKE_CXX_COMPILER_VERSION} (>= ${COMPILER_CLANG_VERSION_MINIMUM} X)")
    else()
        message(STATUS "Clang version: ${CMAKE_CXX_COMPILER_VERSION} (>= ${COMPILER_CLANG_VERSION_MINIMUM} ✓)")
    endif()
elseif (MSVC)
    set(COMPILER_MSVC_VERSION_MINIMUM 1934)
    if (MSVC_VERSION LESS ${COMPILER_MSVC_VERSION_MINIMUM})
        message(FATAL_ERROR "MSVC version too old for C++ Modules.\nMSVC version: ${MSVC_VERSION} (>= ${COMPILER_MSVC_VERSION_MINIMUM} X)")
    else()
        message(STATUS "MSVC version: ${MSVC_VERSION} (>= ${COMPILER_MSVC_VERSION_MINIMUM} ✓)")
    endif()
endif()

# Library
add_library(mcpplibs-primitives STATIC)

file(GLOB_RECURSE CXX_MODULE_FILES CONFIGURE_DEPENDS
    src/*.cppm
)

target_sources(mcpplibs-primitives
    PUBLIC
        FILE_SET CXX_MODULES TYPE CXX_MODULES FILES
        ${CXX_MODULE_FILES}
    PRIVATE
        src/primitives.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(mcpplibs-primitives PRIVATE -fmodules-ts)
endif()

target_link_libraries(mcpplibs-primitives PRIVATE Threads::Threads)

target_include_directories(mcpplibs-primitives PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Examples
add_subdirectory(examples EXCLUDE_FROM_ALL)

# Testing
option(BUILD_TESTING "Build the testing directories" ON)

if (BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
