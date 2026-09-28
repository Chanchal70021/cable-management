#pragma once

namespace cable_manager
{

/**
 * @brief Error code enumeration.
 *
 * Represents failure modes returned by utility APIs that use
 * std::expected as their return type.
 */
enum class ErrorCode
{
    INVALID_INPUT      = 0, ///< One or more input parameters are empty/invalid.
    STANDARD_EXCEPTION = 1, ///< A std::exception was caught during the operation.
};

} // namespace cable_manager
