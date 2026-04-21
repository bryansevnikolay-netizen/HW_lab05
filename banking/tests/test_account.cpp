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

TEST_F(AccountTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(account->id(), 1);
    EXPECT_EQ(account->GetBalance(), 1000);
}

TEST_F(AccountTest, GetBalanceReturnsCorrectValue) {
    EXPECT_EQ(account->GetBalance(), 1000);
}

TEST_F(AccountTest, ChangeBalanceWorksWhenLocked) {
    account->Lock();
    EXPECT_NO_THROW(account->ChangeBalance(500));
    EXPECT_EQ(account->GetBalance(), 1500);
}

TEST_F(AccountTest, ChangeBalanceThrowsWhenNotLocked) {
    EXPECT_THROW(account->ChangeBalance(500), std::runtime_error);
}

TEST_F(AccountTest, LockThrowsWhenAlreadyLocked) {
    account->Lock();
    EXPECT_THROW(account->Lock(), std::runtime_error);
}

TEST_F(AccountTest, UnlockAllowsRelock) {
    account->Lock();
    account->Unlock();
    EXPECT_NO_THROW(account->Lock());
}

TEST_F(AccountTest, MultipleLockUnlockSequence) {
    for (int i = 0; i < 3; i++) {
        EXPECT_NO_THROW(account->Lock());
        EXPECT_NO_THROW(account->ChangeBalance(100));
        EXPECT_NO_THROW(account->Unlock());
    }
    EXPECT_EQ(account->GetBalance(), 1300);
}
