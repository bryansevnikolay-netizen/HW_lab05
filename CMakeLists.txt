cmake_minimum_required(VERSION 3.10)
project(banking)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

add_library(banking STATIC
    Account.cpp
    Transaction.cpp
)

target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(banking PRIVATE -O0 -g --coverage)
        target_link_options(banking PRIVATE --coverage)
    endif()
endif()

enable_testing()
add_subdirectory(tests)
