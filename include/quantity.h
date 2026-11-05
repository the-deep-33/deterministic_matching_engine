#ifndef QUANTITY_H
#define QUANTITY_H

#include <cstdint>
#include <compare>
#include <type_traits>

class Quantity
{
public:
    using Rep = std::int32_t;
private:
    Rep m_qty {};
public:
    explicit Quantity(Rep qty) : m_qty {qty} {}
    Quantity() = default;
    auto operator<=> (const Quantity&) const = default;
    Quantity& operator-= (const Quantity& other)
    {
        this->m_qty -= other.m_qty;
        return (*this);
    }
    Quantity& operator+= (const Quantity& other)
    {
        this->m_qty += other.m_qty;
        return (*this);
    }
    Rep getQuantity() const { return m_qty; }
};

static_assert(std::is_trivially_copyable_v<Quantity>, "Quantity must be trivially copyable!");

#endif