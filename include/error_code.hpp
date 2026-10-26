#pragma once

namespace cable
{

/**
 * @brief Error code enumeration.
 *
 * Represents failure modes returned by utility APIs that use
 * std::expected as their return type.
 */
enum class ErrorCode
{
    INVALID_INPUT = 0,    ///< One or more input parameters are empty/invalid.
    STANDARD_EXCEPTION ,                ///< A std::exception was caught during the operation.
    GPIO_LINE_NOT_FOUND ,                ///< Failed to read a GPIO line.
    /// Near-end and far-end presence pins disagree.
    GPIO_READ_MISMATCH ,
    CABLE_WRONGLY_CONNECTED , ///< Cable present but left present pin de-asserted.
    FAR_END_FAILED ,
    NEAR_END_FAILED,
    CABLE_STATUS_UNKNOWN,
};

} // namespace cable
