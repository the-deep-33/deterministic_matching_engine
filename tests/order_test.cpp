#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include "order.h"

static void expectFreshSlot(const Order& o)
{
    EXPECT_EQ(o.generation, 0u);
    EXPECT_EQ(o.in_use, 0u);
    EXPECT_EQ(o.side, 0u);
    EXPECT_EQ(o.price, Price{0});
    EXPECT_EQ(o.remaining_qty, 0);
    EXPECT_EQ(o.filled_qty, 0);
    EXPECT_EQ(o.participant_id, 0);
    EXPECT_EQ(o.order_prev, kNoSlot);
    EXPECT_EQ(o.order_next, kNoSlot);
}

TEST(Defaults, SingleOrder)
{
    Order o;
    expectFreshSlot(o);
}

TEST(Defaults, HeapArray)
{
    // making sure that the heap array is properly zero initialized.
    constexpr std::size_t kCount = 16;
    auto slots = std::make_unique_for_overwrite<Order[]>(kCount);
    for (std::size_t i = 0; i < kCount; ++i)
    {
        SCOPED_TRACE(i);
        expectFreshSlot(slots[i]);
    }
}

TEST(Layout, HeapArrayIsAligned)
{
    // Big enough that glibc serves it with mmap, where a plain new[] lands 16 bytes past a line.
    constexpr std::size_t kCount = std::size_t{1} << 20;
    auto slots = std::make_unique_for_overwrite<Order[]>(kCount);
    auto address = reinterpret_cast<std::uintptr_t>(slots.get());
    EXPECT_EQ(address % 32, 0u);
}