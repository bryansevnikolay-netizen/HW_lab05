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

    class TestableTransaction : public Transaction {
    public:
        bool saveCalled = false;
    protected:
	void SaveToDataBase(Account& from, Account& to, int sum) override {
            saveCalled = true;
	}
    };

    MockAccount* to;
    MockAccount* from;
    Transaction transaction;
};

TEST_F(TransactionTest, ConstructorSetsDefaultFee) {
    Transaction t;
    EXPECT_EQ(t.fee(), 1);
}

TEST_F(TransactionTest, SetFeeWorksCorrectly) {
    transaction.set_fee(20);
    EXPECT_EQ(transaction.fee(), 20);
}

TEST_F(TransactionTest, MakeThrowsWhenSameAccount) {
    EXPECT_THROW(transaction.Make(*from, *from, 100), std::logic_error);
}

TEST_F(TransactionTest, MakeThrowWhenNegativeSum) {
    EXPECT_THROW(transaction.Make(*from, *to, -50), std::invalid_argument);
}

TEST_F(TransactionTest, MakeThrowsWhenSumLessThan100) {
    EXPECT_THROW(transaction.Make(*from, *to, 50), std::logic_error);
}

TEST_F(TransactionTest, MakeReturnsFalseWhenFeeTooHigh) {
    transaction.set_fee(100);
    EXPECT_FALSE(transaction.Make(*from, *to, 150));
}

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


