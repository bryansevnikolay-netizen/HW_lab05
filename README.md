## Laboratory work V

## Homework

### Задание
1. Создайте `CMakeList.txt` для библиотеки *banking*.
```
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
```
2. Создайте модульные тесты на классы `Transaction` и `Account`.
    Cоздаем CMakeLists для тестов:
```
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
```
    создание mock-файла, содержащего объекты, его методы:
```
#pragma once

#include <gmock/gmock.h>
#include "Account.h"

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(int, GetBalance, (), (const, override));
    MOCK_METHOD(void, ChangeBalance, (int diff), (override));
    MOCK_METHOD(void, Lock, (), (override));
    MOCK_METHOD(void, Unlock, (), (override));
};
```
    Account:
```
#include <gtest/gtest.h>
#include <stdexcept>
#include "Account.h"

class AccountTest : public ::testing::Test {
protected:
    void SetUp() override {
        account = new Account(1, 1000);
    }
    void TearDown() override {
        delete account;
    }
    Account* account;
};
```

```
TEST_F(AccountTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(account->id(), 1);
    EXPECT_EQ(account->GetBalance(), 1000);
}
```

```
TEST_F(AccountTest, GetBalanceReturnsCorrectValue) {
    EXPECT_EQ(account->GetBalance(), 1000);
}
```

```
TEST_F(AccountTest, ChangeBalanceWorksWhenLocked) {
    account->Lock();
    EXPECT_NO_THROW(account->ChangeBalance(500));
    EXPECT_EQ(account->GetBalance(), 1500);
}
```

```
TEST_F(AccountTest, ChangeBalanceThrowsWhenNotLocked) {
    EXPECT_THROW(account->ChangeBalance(500), std::runtime_error);
}
```

```
TEST_F(AccountTest, LockThrowsWhenAlreadyLocked) {
    account->Lock();
    EXPECT_THROW(account->Lock(), std::runtime_error);
}
```

```
TEST_F(AccountTest, UnlockAllowsRelock) {
    account->Lock();
    account->Unlock();
    EXPECT_NO_THROW(account->Lock());
}
```

```
TEST_F(AccountTest, MultipleLockUnlockSequence) {
    for (int i = 0; i < 3; i++) {
        EXPECT_NO_THROW(account->Lock());
        EXPECT_NO_THROW(account->ChangeBalance(100));
        EXPECT_NO_THROW(account->Unlock());
    }
    EXPECT_EQ(account->GetBalance(), 1300);
}
```

    Transaction:
```
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include "Transaction.h"
#include "mock_account.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;

class TransactionTest : public ::testing::Test {
protected:
    void SetUp() override {
	from = new MockAccount(1, 1000);
	to = new MockAccount(2, 500);
	transaction.set_fee(10);
    }

    void TearDown() override {
	delete from;
	delete to;
    }
```

```
    class TestableTransaction : public Transaction {
    public:
        bool saveCalled = false;
    protected:
	void SaveToDataBase(Account& from, Account& to, int sum) override {
            saveCalled = true;
	}
    };
```

```
    MockAccount* to;
    MockAccount* from;
    Transaction transaction;
};
```

```
TEST_F(TransactionTest, ConstructorSetsDefaultFee) {
    Transaction t;
    EXPECT_EQ(t.fee(), 1);
}
```

```
TEST_F(TransactionTest, SetFeeWorksCorrectly) {
    transaction.set_fee(20);
    EXPECT_EQ(transaction.fee(), 20);
}
```

```
TEST_F(TransactionTest, MakeThrowsWhenSameAccount) {
    EXPECT_THROW(transaction.Make(*from, *from, 100), std::logic_error);
}
```

```
TEST_F(TransactionTest, MakeThrowWhenNegativeSum) {
    EXPECT_THROW(transaction.Make(*from, *to, -50), std::invalid_argument);
}
```

```
TEST_F(TransactionTest, MakeThrowsWhenSumLessThan100) {
    EXPECT_THROW(transaction.Make(*from, *to, 50), std::logic_error);
}
```

```
TEST_F(TransactionTest, MakeReturnsFalseWhenFeeTooHigh) {
    transaction.set_fee(100);
    EXPECT_FALSE(transaction.Make(*from, *to, 150));
}
```

```
TEST_F(TransactionTest, MakeSucceedWhenSufficientFunds) {
    EXPECT_CALL(*from, Lock()).Times(1);
    EXPECT_CALL(*to, Lock()).Times(1);
    EXPECT_CALL(*to, ChangeBalance(150)).Times(1);
    EXPECT_CALL(*from, GetBalance()).WillRepeatedly(Return(1000));
    EXPECT_CALL(*from, ChangeBalance(-160)).Times(1);
    EXPECT_CALL(*from, Unlock()).Times(1);
    EXPECT_CALL(*to, Unlock()).Times(1);

    EXPECT_TRUE(transaction.Make(*from, *to, 150));
}
```

```
TEST_F(TransactionTest, MakeRollsBackWhenInsufficientFunds) {
    EXPECT_CALL(*from, Lock()).Times(1);
    EXPECT_CALL(*to, Lock()).Times(1);
    EXPECT_CALL(*to, ChangeBalance(150)).Times(1);
    EXPECT_CALL(*from, GetBalance()).WillRepeatedly(Return(100));
    EXPECT_CALL(*from, ChangeBalance(-160)).Times(0);
    EXPECT_CALL(*to, ChangeBalance(-150)).Times(1);
    EXPECT_CALL(*from, Unlock()).Times(1);
    EXPECT_CALL(*to, Unlock()).Times(1);

    EXPECT_FALSE(transaction.Make(*from, *to, 150));
}
```

```
TEST_F(TransactionTest, SaveToDataBaseIsCalled) {
    TestableTransaction testTransaction;
    testTransaction.set_fee(10);

    EXPECT_CALL(*from, Lock()).Times(1);
    EXPECT_CALL(*to, Lock()).Times(1);
    EXPECT_CALL(*to, ChangeBalance(150)).Times(1);
    EXPECT_CALL(*from, GetBalance()).WillRepeatedly(Return(1000));
    EXPECT_CALL(*from, ChangeBalance(-160)).Times(1);
    EXPECT_CALL(*from, Unlock()).Times(1);
    EXPECT_CALL(*to, Unlock()).Times(1);

    testTransaction.Make(*from, *to, 150);

    EXPECT_TRUE(testTransaction.saveCalled);
}
```

3. Настройте сборочную процедуру на **TravisCI**.
```
name: Build, Test and Coverage

on:
  push:
    branches: [ main, master ]
  pull_request:
    branches: [ main, master ]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout code
        uses: actions/checkout@v4
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y lcov
      - name: Install Google Test and Mock
        run: |
          cd /tmp
          git clone https://github.com/google/googletest.git
          cd googletest
          mkdir build && cd build
          cmake -DCMAKE_CXX_STANDARD=17 ..
          make -j$(nproc)
          sudo make install
          sudo ldconfig
      - name: Configure CMake with coverage
        run: |
          mkdir -p build
          cd build
          cmake .. -DENABLE_COVERAGE=ON
      - name: Build
        run: |
          cd build
          make
      - name: Run tests
        run: |
          cd build
          ctest --output-on-failure
      - name: Collect coverage
        run: |
          cd build
          lcov --directory . --capture --output-file coverage.info --ignore-errors mismatch
          lcov --remove coverage.info '/usr/*' --output-file coverage.info --ignore-errors mismatch
          lcov --remove coverage.info '*/tests/*' --output-file coverage.info --ignore-errors mismatch
          lcov --list coverage.info
      - name: Upload to Coveralls
        uses: coverallsapp/github-action@v2
        with:
          file: build/coverage.info
          format: lcov
          
```
4. Настройте [Coveralls.io](https://coveralls.io/).


