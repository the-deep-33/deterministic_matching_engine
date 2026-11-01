#include <gtest/gtest.h>
#include <cstdlib>

TEST(SuccessTest, OneEqOne)
{
    EXPECT_EQ(1, 1);
}

TEST(DeathTest, Abort)
{
    EXPECT_DEATH(std::abort(), "");
}