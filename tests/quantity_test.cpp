#include <gtest/gtest.h>
#include <algorithm>
#include <limits>
#include "quantity.h"

TEST(QuantityDefaults, DefaultIsZero)
{
    Quantity q;
    EXPECT_EQ(q.getQuantity(), 0);
}

TEST(QuantityGetter, QuantityMember)
{
    Quantity q {250};
    EXPECT_EQ(q.getQuantity(), 250);
}

TEST(QuantityGetter, LimitsRoundTrip)
{
    Quantity high {std::numeric_limits<Quantity::Rep>::max()};
    Quantity low {std::numeric_limits<Quantity::Rep>::min()};
    EXPECT_EQ(high.getQuantity(), std::numeric_limits<Quantity::Rep>::max());
    EXPECT_EQ(low.getQuantity(), std::numeric_limits<Quantity::Rep>::min());
    EXPECT_LT(low, high);
}

TEST(QuantityCompare, LessThan)
{
    Quantity q1 {20};
    Quantity q2 {40};
    EXPECT_LT(q1, q2);
    EXPECT_FALSE(q2 < q1);
    EXPECT_FALSE(q1 < q1);
}

TEST(QuantityCompare, GreaterThan)
{
    Quantity q1 {40};
    Quantity q2 {20};
    EXPECT_GT(q1, q2);
    EXPECT_FALSE(q2 > q1);
    EXPECT_FALSE(q1 > q1);
}

TEST(QuantityCompare, Equals)
{
    Quantity q1 {20};
    Quantity q2 {20};
    EXPECT_EQ(q1, q2);
    EXPECT_FALSE(q1 != q2);
}

TEST(QuantityCompare, NotEquals)
{
    Quantity q1 {20};
    Quantity q2 {40};
    EXPECT_NE(q1, q2);
    EXPECT_FALSE(q1 == q2);
}

TEST(QuantityArithmetic, PlusEquals)
{
    Quantity q {10};
    q += Quantity {5};
    EXPECT_EQ(q.getQuantity(), 15);
}

TEST(QuantityArithmetic, MinusEquals)
{
    Quantity q {10};
    q -= Quantity {4};
    EXPECT_EQ(q.getQuantity(), 6);
}

TEST(QuantityArithmetic, MinusEqualsGoesNegative)
{
    Quantity q {3};
    q -= Quantity {5};
    EXPECT_EQ(q.getQuantity(), -2);
}

TEST(QuantityMin, PicksTheSmaller)
{
    Quantity q1 {7};
    Quantity q2 {3};
    EXPECT_EQ(std::min(q1, q2), q2);
    EXPECT_EQ(std::min(q2, q1), q2);
}

TEST(QuantityFill, OneFill)
{
    Quantity incoming {4};
    Quantity resting {10};
    Quantity filled;

    Quantity fill = std::min(incoming, resting);
    incoming -= fill;
    resting -= fill;
    filled += fill;

    EXPECT_EQ(incoming.getQuantity(), 0);
    EXPECT_EQ(resting.getQuantity(), 6);
    EXPECT_EQ(filled.getQuantity(), 4);
}
