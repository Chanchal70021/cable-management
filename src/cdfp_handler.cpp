#include "cdfp_handler.hpp"

#include "constants.hpp"
#include "utils.hpp"

#include <phosphor-logging/lg2.hpp>

namespace cable
{

std::expected<bool, ErrorCode> CdfpHandler::leftCableDetected() noexcept
{
    const auto nearEndRes =
        utils::readGpioVal(constants::cdfp0GpioNearEndPresN);
    if (!nearEndRes)
    {
        lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                   constants::cdfp0GpioNearEndPresN, "ERR",
                   utils::getErrorCodeMsg(nearEndRes.error()));
        return std::unexpected(ErrorCode::NEAR_END_FAILED);
    }

    const auto farEndRes = utils::readGpioVal(constants::cdfp0GpioFarEndPresN);
    if (!farEndRes)
    {
        lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                   constants::cdfp0GpioFarEndPresN, "ERR",
                   utils::getErrorCodeMsg(farEndRes.error()));
            return std::unexpected(ErrorCode::FAR_END_FAILED);
    }

    // Active-low: raw 0 → pin asserted → cable end present.
    const bool nearEndPresent = (nearEndRes.value() == 0);
    const bool farEndPresent = (farEndRes.value() == 0);

    if (!nearEndPresent && !farEndPresent)
    {
        lg2::info("Near-end and far-end both absent – cable not present "
                  "{NE_PIN}={NE}, {FE_PIN}={FE}",
                  "NE_PIN", constants::cdfp0GpioNearEndPresN, "NE",
                  nearEndRes.value(), "FE_PIN", constants::cdfp0GpioFarEndPresN,
                  "FE", farEndRes.value());
        return false;
    }

    if (nearEndPresent && !farEndPresent)
    {
        return std::unexpected(ErrorCode::FAR_END_FAILED);
    }

    if (!nearEndPresent && farEndPresent)
    {
        return std::unexpected(ErrorCode::NEAR_END_FAILED);
    }

    const auto leftPresentRes =
        utils::readGpioVal(constants::cdfp0GpioPresLeftN);
    if (!leftPresentRes)
    {
        lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                   constants::cdfp0GpioPresLeftN, "ERR",
                   utils::getErrorCodeMsg(leftPresentRes.error()));
        return std::unexpected(ErrorCode::CABLE_STATUS_UNKNOWN);
    }

    // Active-low: raw 0 → cable correctly seated.
    // raw 1 (de-asserted) → cable wrongly connected / mis-seated.
    if (leftPresentRes.value() != 0)
    {
        return std::unexpected(ErrorCode::CABLE_WRONGLY_CONNECTED);
    }

    return true;
}

} // namespace cable
