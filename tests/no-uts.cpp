#include <iostream>
#include <gtest/gtest.h>
#include <map>

int factorial(int n) {
    if(n == 0 || n == 1) {
        return 1;
    } else {
        return n * factorial(n-1);
    }
}

/* basic tests*/
TEST(FactorialTest, HandlesZeroInput) {
    EXPECT_EQ(factorial(0), 1);
}

TEST(FactorialTest, HandlesPositiveInput) {
    EXPECT_EQ(factorial(1), 1);
    EXPECT_EQ(factorial(2), 2);
    EXPECT_EQ(factorial(3), 6);
    EXPECT_EQ(factorial(4), 24);
    EXPECT_EQ(factorial(5), 120);
}

/*tests with test data*/
class FactorialTestF : public testing::Test {
protected:
    std::map<int, int> kvpairs;
public:
    FactorialTestF() : kvpairs({{0,1}, {1,1}, {2,2}, {3,6}, {4,24}, {5,120}}) {}
};

TEST_F(FactorialTestF, HandlesAllPreviousInputButByFixture) {
    for(auto& kv : kvpairs) {
        EXPECT_EQ(factorial(kv.first), kv.second);
    }
}

/* some utility assertions*/
TEST(BullshitTest, HandlesAnException) {
    EXPECT_THROW({
        throw std::exception();
    }, std::exception);
}

TEST(BullshitTest, HandlesAnotherException) {
    EXPECT_ANY_THROW({
        throw std::exception();
    });
}


/*tests organized by common test data*/
class FactorialTestFParam : public testing::TestWithParam<int> {
protected:
    std::map<int, int> kvpairs;
public:
    FactorialTestFParam() : kvpairs({{0,1}, {1,1}, {2,2}, {3,6}, {4,24}, {5,120}}) {}
};

TEST_P(FactorialTestFParam, AgainTheSameShit) {
    int n = GetParam();
    EXPECT_EQ(factorial(n), kvpairs[n]);
}

INSTANTIATE_TEST_CASE_P(FactorialTestF, FactorialTestFParam, testing::Values(0, 1, 2, 3, 4, 5));

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}