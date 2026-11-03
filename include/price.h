#ifndef PRICE_H
#define PRICE_H

#include <cstdint>
#include <compare>
#include <type_traits>

class Price
{
public:
    using Rep = std::int32_t;
private:
    Rep m_price {};
public:
    explicit Price(Rep price) : m_price {price} {}
    Price() = default;
    auto operator<=> (const Price&) const = default;
    Rep getPrice() const { return m_price; }

};

static_assert(std::is_trivially_copyable_v<Price>, "Price must be trivially copyable!");

#endif