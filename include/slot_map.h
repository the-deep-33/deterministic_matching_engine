#ifndef SLOT_MAP_H
#define SLOT_MAP_H

#include <cstdint>
#include <cassert>
#include <memory>
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
        assert(!full());
        --m_counter;
        std::uint32_t slot = m_stack[m_counter];
        m_orders[slot].in_use = 1;
        return slot;
    }
    void release(std::uint32_t slot)
    {
        assert(slot < kSlotCount);
        assert(m_orders[slot].in_use == 1);
        m_orders[slot].in_use = 0;
        ++m_orders[slot].generation;
        m_stack[m_counter] = slot;
        ++m_counter;
    }
};

#endif