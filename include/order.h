#ifndef ORDER_H
#define ORDER_H

#include <cstdint>
#include <type_traits>
#include "price.h"
#include "types.h"
#include "quantity.h"

struct alignas(32) Order
{
private:
    std::uint64_t m_generation : kGenerationBits {};
    std::uint64_t m_in_use : 1 {};
    std::uint64_t m_side : 1 {};
    template <std::uint32_t SlotBits> friend class SlotMap;
public:
    Price price {};
    Quantity remaining_qty {};
    Quantity filled_qty {};
    std::int32_t participant_id {}; //ID of the exchange participant who put out the order
    // The next two members have the slot numbers of the previous and the next order, so that we can link them together like a list
    std::uint32_t order_prev {kNoSlot};
    std::uint32_t order_next {kNoSlot};

    std::uint64_t getGeneration() const { return m_generation; }
    bool getInUse() const { return m_in_use == 1; }
    Side getSide() const { return m_side == 0 ? Side::Buy : Side::Sell; }
    void setSide(Side side) { m_side = (side == Side::Sell); }
};

static_assert(std::is_trivially_copyable_v<Order>, "Order must be trivially copyable!");
static_assert(sizeof(Order) == 32, "Order should be 32 bytes.");
static_assert(alignof(Order) == 32, "Order should begin at k*32 memory slot for cache friendliness");

#endif