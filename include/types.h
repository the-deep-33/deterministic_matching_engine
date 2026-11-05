#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <limits>

enum class Side: std::uint8_t {Buy, Sell};
inline constexpr std::uint32_t kNoSlot          = std::numeric_limits<std::uint32_t>::max(); 
inline constexpr std::uint32_t kGenerationBits  = 44; // 44 bits to save the generation (how many times a slot was reused), the other 20 are going in to save the slot number

#endif