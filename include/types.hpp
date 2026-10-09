#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <variant>

namespace cable::types {

// Define a common Dbus variant type
using DbusVariantType =
    std::variant<bool, uint32_t, uint64_t, std::string>;


/**
 * @brief Represents the physical position of a BMC
 *
 * POSITION_0 and POSITION_1 are the two valid slots.
 * INVALID_VALUE is returned when the position cannot be determined.
 */
enum class BmcPosition : size_t {
  POSITION_0 = 0,
  POSITION_1 = 1,
  INVALID_VALUE = std::numeric_limits<size_t>::max()
};

} // namespace cable::types
