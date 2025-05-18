# TODO: check minimal version https://github.com/nlohmann/cmake_min_version
cmake_minimum_required(VERSION 3.10)
project(hoblang)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DISABLE_TESTS "Disable tests generation" OFF)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)
find_package(LLVM REQUIRED)
message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")

add_compile_options(-O3 -g -fsanitize=address -Wall -Wextra -Werror)
add_link_options(-fsanitize=address)

file(GLOB_RECURSE sources src/*.c)
list(FILTER sources EXCLUDE REGEX "\\.test\\.")
add_executable(hoblang ${sources} hoblang.c)
target_link_libraries(hoblang PRIVATE LLVM)
include_directories(hoblang src)

if (NOT DISABLE_TESTS)
    find_library(CUNIT_LIBRARY NAMES
        cunit
        libcunit
        cunitlib
    )
    mark_as_advanced(CUNIT_LIBRARY)

    file(GLOB_RECURSE test_sources src/*.c)
    add_executable(hoblang-tests ${test_sources} tests.c)
    include_directories(hoblang-tests src)
    target_link_libraries(hoblang-tests ${CUNIT_LIBRARY} LLVM)
    file(COPY for_tests DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
