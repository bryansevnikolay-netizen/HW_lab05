find_package(GTest REQUIRED)

add_executable(banking_tests
    test_account.cpp
    test_transaction.cpp
    mock_account.h
)

target_link_libraries(banking_tests
    banking
    /usr/local/lib/libgtest.a
    /usr/local/lib/libgtest_main.a
    /usr/local/lib/libgmock.a
    pthread
)

target_include_directories(banking_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/local/include
)

add_test(NAME BankingTests COMMAND banking_tests)

if(ENABLE_COVERAGE)
    target_compile_options(banking_tests PRIVATE -O0 -g --coverage)
    target_link_options(banking_tests PRIVATE --coverage)
endif()
