#include <gtest/gtest.h>
#include <limits>
#include "price.h"

TEST(Compare, LessThan)
{
    Price p1 {20};
    Price p2 {40};
    EXPECT_LT(p1, p2);
    EXPECT_FALSE(p2 < p1);
    EXPECT_FALSE(p1 < p1);
}

TEST(Compare, GreaterThan)
{
    Price p1 {40};
    Price p2 {20};
    EXPECT_GT(p1, p2);
    EXPECT_FALSE(p2 > p1);
    EXPECT_FALSE(p1 > p1);
}

TEST(Compare, LessOrEqual)
{
    Price p1 {20};
    Price p2 {40};
    Price p3 {20};
    EXPECT_LE(p1, p2);
    EXPECT_LE(p1, p3);
    EXPECT_FALSE(p2 <= p1);
}

TEST(Compare, GreaterOrEqual)
{
    Price p1 {40};
    Price p2 {20};
    Price p3 {40};
    EXPECT_GE(p1, p2);
    EXPECT_GE(p1, p3);
    EXPECT_FALSE(p2 >= p1);
}

TEST(Compare, Equals)
{
    Price p1 {20};
    Price p2 {20};
    EXPECT_EQ(p1, p2);
    EXPECT_FALSE(p1 != p2);
}

TEST(Compare, NotEquals)
{
    Price p1 {20};
    Price p2 {40};
    EXPECT_NE(p1, p2);
    EXPECT_FALSE(p1 == p2);
}

TEST(Getter, PriceMember)
{
    Price p1 {20};
    Price::Rep price = p1.getPrice();
    EXPECT_EQ(price, 20);
}

TEST(Getter, LimitsRoundTrip)
{
    Price high {std::numeric_limits<Price::Rep>::max()};
    Price low {std::numeric_limits<Price::Rep>::min()};
    EXPECT_EQ(high.getPrice(), std::numeric_limits<Price::Rep>::max());
    EXPECT_EQ(low.getPrice(), std::numeric_limits<Price::Rep>::min());
    EXPECT_LT(low, high);
}