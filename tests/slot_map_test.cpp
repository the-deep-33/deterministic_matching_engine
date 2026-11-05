#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
#include "slot_map.h"

TEST(SlotMapAllocate, FreshMapHandsOutSlotsInOrder)
{
    SlotMap<2> map;
    EXPECT_EQ(map.allocate(), 0u);
    EXPECT_EQ(map.allocate(), 1u);
    EXPECT_EQ(map.allocate(), 2u);
    EXPECT_FALSE(map.full());
    EXPECT_EQ(map.allocate(), 3u);
    EXPECT_TRUE(map.full());
}

TEST(SlotMapAllocate, LastInFirstOutCheck)
{
    SlotMap<2> map;
    map.allocate();
    map.allocate();
    map.allocate();

    map.release(1);
    map.release(0);
    map.release(2);

    EXPECT_EQ(map.allocate(), 2u);
    EXPECT_EQ(map.allocate(), 0u);
    EXPECT_EQ(map.allocate(), 1u);
}

TEST(SlotMapAllocate, ReleaseMakesFullMapUsableAgain)
{
    SlotMap<2> map;
    for (int i = 0; i < 4; ++i)
        map.allocate();
    ASSERT_TRUE(map.full());

    map.release(1);
    EXPECT_FALSE(map.full());
    EXPECT_EQ(map.allocate(), 1u);
    EXPECT_TRUE(map.full());
}

TEST(SlotMapDeathTest, AllocateWhenFull)
{
#ifdef NDEBUG
    GTEST_SKIP() << "assert is compiled out in Release";
#endif
    SlotMap<2> map;
    for (int i = 0; i < 4; ++i)
        map.allocate();
    EXPECT_DEATH(map.allocate(), "full");
}

TEST(SlotMapDeathTest, DoubleRelease)
{
#ifdef NDEBUG
    GTEST_SKIP() << "assert is compiled out in Release";
#endif
    SlotMap<2> map;
    std::uint32_t slot = map.allocate();
    map.release(slot);
    EXPECT_DEATH(map.release(slot), "in_use");
}

TEST(SlotMapDeathTest, ReleaseNeverAllocated)
{
#ifdef NDEBUG
    GTEST_SKIP() << "assert is compiled out in Release";
#endif
    SlotMap<2> map;
    EXPECT_DEATH(map.release(2), "in_use");
}

TEST(SlotMapRandom, NeverHandsOutATakenSlot)
{
    constexpr std::size_t kSlots = 16;
    SlotMap<4> map;
    std::vector<bool> taken(kSlots, false);
    std::vector<std::uint32_t> live;

    std::mt19937 rng {12345};
    std::bernoulli_distribution coin {0.5};

    for (int step = 0; step < 10000; ++step)
    {
        bool do_allocate = live.empty() || (!map.full() && coin(rng));
        if (do_allocate)
        {
            std::uint32_t slot = map.allocate();
            ASSERT_LT(slot, kSlots);
            ASSERT_FALSE(taken[slot]) << "slot " << slot << " handed out twice, step " << step;
            taken[slot] = true;
            live.push_back(slot);
        }
        else
        {
            std::uniform_int_distribution<std::size_t> pick {0, live.size() - 1};
            std::size_t i = pick(rng);
            std::uint32_t slot = live[i];
            live[i] = live.back();
            live.pop_back();
            map.release(slot);
            taken[slot] = false;
        }
        ASSERT_EQ(map.full(), live.size() == kSlots) << "step " << step;
    }
}
