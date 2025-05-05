# TODO: check minimal version https://github.com/nlohmann/cmake_min_version
cmake_minimum_required(VERSION 3.10)
project(hoblang)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DISABLE_TESTS "Disable tests generation" OFF)

file(GLOB_RECURSE sources src/*.c)
list(FILTER sources EXCLUDE REGEX "\\.test\\.")
add_executable(hoblang ${sources} hoblang.c)
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
    set_target_properties(hoblang-tests PROPERTIES LINK_FLAGS -fsanitize=address)
    set_target_properties(hoblang-tests PROPERTIES COMPILE_FLAGS -fsanitize=address)
    include_directories(hoblang-tests src)
    target_link_libraries(hoblang-tests ${CUNIT_LIBRARY})
    file(COPY for_tests DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
