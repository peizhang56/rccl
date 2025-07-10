#include "bitops.h"

#include "gtest/gtest.h"

using namespace ::testing;

class BitOpsTestsFixture : public Test {
public:
    BitOpsTestsFixture() = default;
    ~BitOpsTestsFixture() override = default;
protected:
    int m_size{0};
};


TEST_F(BitOpsTestsFixture, DIVUP_Tests) {
    EXPECT_EQ(DIVUP(0, 8), 0);
    EXPECT_EQ(DIVUP(1, 8), 1);
    EXPECT_EQ(DIVUP(7, 8), 1);
    EXPECT_EQ(DIVUP(8, 8), 1);
    EXPECT_EQ(DIVUP(9, 8), 2);
}

TEST_F(BitOpsTestsFixture, ROUNDUP_Tests) {
    EXPECT_EQ(ROUNDUP(0, 8), 0);
    EXPECT_EQ(ROUNDUP(1, 8), 8);
    EXPECT_EQ(ROUNDUP(7, 8), 8);
    EXPECT_EQ(ROUNDUP(8, 8), 8);
    EXPECT_EQ(ROUNDUP(9, 8), 16);
    EXPECT_EQ(ROUNDUP(15, 8), 16);
    EXPECT_EQ(ROUNDUP(16, 8), 16);
    EXPECT_EQ(ROUNDUP(17, 8), 24);
}

TEST_F(BitOpsTestsFixture, ALIGN_POWER_Tests) {
    EXPECT_EQ(ALIGN_POWER(7, 8), 8);
    EXPECT_EQ(ALIGN_POWER(8, 8), 8);
    EXPECT_EQ(ALIGN_POWER(9, 8), 16);
}

TEST_F(BitOpsTestsFixture, ALIGN_SIZE_Tests) {
    m_size = 1;
    ALIGN_SIZE(m_size, 8);
    EXPECT_EQ(m_size, 8);
    m_size = 7;
    ALIGN_SIZE(m_size, 8);
    EXPECT_EQ(m_size, 8);
}

TEST_F(BitOpsTestsFixture, divUp_Tests) {
    EXPECT_EQ(divUp(0, 8), 0);
    EXPECT_EQ(divUp(1, 8), 1);
    EXPECT_EQ(divUp(7, 8), 1);
    EXPECT_EQ(divUp(8, 8), 1);
    EXPECT_EQ(divUp(9, 8), 2);
    EXPECT_EQ(divUp(15, 8), 2);
    EXPECT_EQ(divUp(16, 8), 2);
    EXPECT_EQ(divUp(17, 8), 3);
}

TEST_F(BitOpsTestsFixture, roundUp_Tests) {
    EXPECT_EQ(roundUp(0, 8), 0);
    EXPECT_EQ(roundUp(1, 8), 8);
    EXPECT_EQ(roundUp(7, 8), 8);
    EXPECT_EQ(roundUp(8, 8), 8);
    EXPECT_EQ(roundUp(9, 8), 16);
    EXPECT_EQ(roundUp(15, 8), 16);
    EXPECT_EQ(roundUp(16, 8), 16);
    EXPECT_EQ(roundUp(17, 8), 24);
}

TEST_F(BitOpsTestsFixture, roundDown_Tests) {
    EXPECT_EQ(roundDown(0, 8), 0);
    EXPECT_EQ(roundDown(1, 8), 0);
    EXPECT_EQ(roundDown(7, 8), 0);
    EXPECT_EQ(roundDown(8, 8), 8);
    EXPECT_EQ(roundDown(9, 8), 8);
    EXPECT_EQ(roundDown(15, 8), 8);
    EXPECT_EQ(roundDown(16, 8), 16);
    EXPECT_EQ(roundDown(17, 8), 16);
}

TEST_F(BitOpsTestsFixture, alignUp_Tests) {
    EXPECT_EQ(alignUp(0, 8), 0);
    EXPECT_EQ(alignUp(1, 8), 8);
    EXPECT_EQ(alignUp(7, 8), 8);
    EXPECT_EQ(alignUp(8, 8), 8);
    EXPECT_EQ(alignUp(9, 8), 16);
    EXPECT_EQ(alignUp(15, 8), 16);
    EXPECT_EQ(alignUp(16, 8), 16);
    EXPECT_EQ(alignUp(17, 8), 24);
}