#ifndef SLOT_MAP_H
#define SLOT_MAP_H

#include <cstdint>
#include <memory>
#include <iostream>
#include <string_view>
#include <cstdlib>
#include "order.h"
#include "types.h"

template <std::uint32_t SlotBits>
class SlotMap
{
    static_assert(SlotBits + kGenerationBits <= 64, "slot bits plus generation bits must fit in a 64-bit id");
private:
    static constexpr std::uint32_t kSlotCount   = std::uint32_t{1} << SlotBits;
    std::unique_ptr<Order []> m_orders          = std::make_unique_for_overwrite<Order []>(kSlotCount);
    std::unique_ptr<std::uint32_t []> m_stack   = std::make_unique_for_overwrite<std::uint32_t []>(kSlotCount);
    std::uint32_t m_counter {kSlotCount};
    [[noreturn]] void slotMapFailure(std::string_view msg) const
    {
        std::cerr << "Slot map: " << msg << '\n';
        std::abort();
    }
public:
    SlotMap()
    {
        for (std::uint32_t i {0}; i < kSlotCount; ++i)
        {
            m_stack[i] = kSlotCount - i - 1;
        }
    }

    bool full() const { return m_counter == 0; }

    std::uint32_t allocate()
    {
        if(full()) [[unlikely]]
        {
            slotMapFailure("Allocating on a full map");
        }
        --m_counter;
        std::uint32_t slot = m_stack[m_counter];
        m_orders[slot].m_in_use = 1;
        return slot;
    }

    void release(std::uint32_t slot)
    {
        if(slot >= kSlotCount) [[unlikely]]
        {
            slotMapFailure("Invalid slot, slot >= max slot number");
        }
        if(m_orders[slot].m_in_use == 0) [[unlikely]]
        {
            slotMapFailure("This slot is vacant");
        }
        Order fresh {};
        fresh.m_generation = m_orders[slot].m_generation;
        m_orders[slot] = fresh;
        ++m_orders[slot].m_generation;
        m_stack[m_counter] = slot;
        ++m_counter;
    }


    Order& get(std::uint32_t slot)
    {
        if(slot >= kSlotCount) [[unlikely]]
        {
            slotMapFailure("Invalid slot, slot >= max slot number");
        }
        return m_orders[slot];
    }
    const Order& get(std::uint32_t slot) const
    {
        if(slot >= kSlotCount) [[unlikely]]
        {
            slotMapFailure("Invalid slot, slot >= max slot number");
        }
        return m_orders[slot];
    }

    SlotMap(SlotMap&&) = delete;
    SlotMap& operator= (SlotMap&&) = delete;
};

#endif