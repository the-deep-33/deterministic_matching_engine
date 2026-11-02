#ifndef ORDER_H
#define ORDER_H

#include <cstdint>
#include <type_traits>
#include "price.h"
#include "types.h"

struct Order
{
    std::uint64_t generation : 40;
    std::uint64_t in_use : 1;
    std::uint64_t side : 1;
    Price price;
    std::int32_t remaining_qty;
    std::int32_t filled_qty;
    std::int32_t participant_id; //ID of the exchange participant who put out the order
    // The next two members have the slot numbers of the previous and the next order, so that we can link them together like a list
    std::uint32_t order_prev;
    std::uint32_t order_next;
};

static_assert(std::is_trivially_copyable_v<Order>, "Order must be trivially copyable!");
static_assert(std::is_trivially_constructible_v<Order>, "Order must be trivially constructible!");
static_assert(sizeof(Order) == 32, "Order should be 32 bytes.");

#endif