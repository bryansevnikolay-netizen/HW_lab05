cmake_minimum_required(VERSION 3.10)
project(banking_tests)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

find_package(GTest REQUIRED)

add_executable(run_tests
    test_account.cpp
    test_transaction.cpp
)

target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(run_tests
    banking
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
)

add_test(NAME BankingTests COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_tests)
