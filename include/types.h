#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <limits>

enum class Side: std::uint8_t {Buy, Sell};
constexpr std::uint32_t kNoSlot = std::numeric_limits<std::uint32_t>::max(); 

#endif