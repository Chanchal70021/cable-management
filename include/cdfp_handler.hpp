#pragma once

#include "error_code.hpp"

#include <expected>

namespace cable
{

/**
 * @brief Groups CDFP cable specific handling tasks.
 */
class CdfpHandler
{
  public:
    CdfpHandler() = delete;
    CdfpHandler(const CdfpHandler&) = delete;
    CdfpHandler& operator=(const CdfpHandler&) = delete;
    CdfpHandler(CdfpHandler&&) = delete;
    CdfpHandler& operator=(CdfpHandler&&) = delete;

    /**
     * @brief Detect whether the left CDFP cable is present.
     *
     * @return true if present, false if absent, or errorCode on failure.
     */
    static std::expected<bool, ErrorCode> leftCableDetected() noexcept;
};

} // namespace cable
