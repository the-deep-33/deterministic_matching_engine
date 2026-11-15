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
    SlotMap<2> map;
    for (int i = 0; i < 4; ++i)
        map.allocate();
    EXPECT_DEATH(map.allocate(), "full");
}

TEST(SlotMapDeathTest, DoubleRelease)
{
    SlotMap<2> map;
    std::uint32_t slot = map.allocate();
    map.release(slot);
    EXPECT_DEATH(map.release(slot), "vacant");
}

TEST(SlotMapDeathTest, ReleaseNeverAllocated)
{
    SlotMap<2> map;
    EXPECT_DEATH(map.release(2), "vacant");
}

TEST(SlotMapDeathTest, ReleaseOutOfRange)
{
    SlotMap<2> map;
    EXPECT_DEATH(map.release(4), "Invalid slot");
}

TEST(SlotMapDeathTest, GetOutOfRange)
{
    SlotMap<2> map;
    EXPECT_DEATH(map.get(4), "Invalid slot");
}

TEST(SlotMapRelease, InUseFollowsAllocateAndRelease)
{
    SlotMap<2> map;
    EXPECT_FALSE(map.get(0).getInUse());

    std::uint32_t slot = map.allocate();
    EXPECT_TRUE(map.get(slot).getInUse());

    map.release(slot);
    EXPECT_FALSE(map.get(slot).getInUse());
}

TEST(SlotMapRelease, ReleasedSlotKeepsNothingFromItsOrder)
{
    SlotMap<2> map;
    std::uint32_t slot = map.allocate();

    Order& order = map.get(slot);
    order.setSide(Side::Sell);
    order.price = Price{6468347};
    order.remaining_qty = Quantity{7};
    order.filled_qty = Quantity{3};
    order.participant_id = 42;
    order.order_prev = 1;
    order.order_next = 2;

    map.release(slot);

    const Order& freed = map.get(slot);
    EXPECT_EQ(freed.getSide(), Side::Buy);
    EXPECT_EQ(freed.price, Price{0});
    EXPECT_EQ(freed.remaining_qty, Quantity{0});
    EXPECT_EQ(freed.filled_qty, Quantity{0});
    EXPECT_EQ(freed.participant_id, 0);
    EXPECT_EQ(freed.order_prev, kNoSlot);
    EXPECT_EQ(freed.order_next, kNoSlot);
}

TEST(SlotMapRelease, GenerationGoesUpByOnePerRelease)
{
    SlotMap<2> map;
    std::uint32_t slot = map.allocate();
    EXPECT_EQ(map.get(slot).getGeneration(), 0u);

    for (std::uint64_t releases = 1; releases <= 3; ++releases)
    {
        map.release(slot);
        EXPECT_EQ(map.get(slot).getGeneration(), releases);

        ASSERT_EQ(map.allocate(), slot);
        EXPECT_EQ(map.get(slot).getGeneration(), releases);
    }
}

TEST(SlotMapRelease, OtherSlotsAreLeftAlone)
{
    SlotMap<2> map;
    std::uint32_t kept = map.allocate();
    std::uint32_t freed = map.allocate();
    map.get(kept).remaining_qty = Quantity{5};

    map.release(freed);

    EXPECT_TRUE(map.get(kept).getInUse());
    EXPECT_EQ(map.get(kept).getGeneration(), 0u);
    EXPECT_EQ(map.get(kept).remaining_qty, Quantity{5});
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
