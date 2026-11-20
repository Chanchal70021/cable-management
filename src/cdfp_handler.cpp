#include "cdfp_handler.hpp"

#include "constants.hpp"
#include "utils.hpp"

#include <phosphor-logging/lg2.hpp>

namespace cable
{

std::expected<bool, ErrorCode> CdfpHandler::cdfp0CableDetected() noexcept
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

std::expected<bool, ErrorCode> CdfpHandler::cdfp1CableDetected() noexcept
{
    const auto nearEndRes =
        utils::readGpioVal(constants::cdfp1GpioNearEndPresN);
    if (!nearEndRes)
    {
        lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                   constants::cdfp1GpioNearEndPresN, "ERR",
                   utils::getErrorCodeMsg(nearEndRes.error()));
        return std::unexpected(ErrorCode::NEAR_END_FAILED);
    }

    const auto farEndRes = utils::readGpioVal(constants::cdfp1GpioFarEndPresN);
    if (!farEndRes)
    {
        lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                   constants::cdfp1GpioFarEndPresN, "ERR",
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
                  "NE_PIN", constants::cdfp1GpioNearEndPresN, "NE",
                  nearEndRes.value(), "FE_PIN", constants::cdfp1GpioFarEndPresN,
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

    const auto rightPresentRes =
        utils::readGpioVal(constants::cdfp1GpioPresRightP);
    if (!rightPresentRes)
    {
        lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                   constants::cdfp1GpioPresRightP, "ERR",
                   utils::getErrorCodeMsg(rightPresentRes.error()));
        return std::unexpected(ErrorCode::CABLE_STATUS_UNKNOWN);
    }

    // Active-high (_P): raw 1 → cable correctly seated.
    // raw 0 (de-asserted) → cable wrongly connected / mis-seated.
    if (rightPresentRes.value() == 0)
    {
        return std::unexpected(ErrorCode::CABLE_WRONGLY_CONNECTED);
    }

    return true;
}

} // namespace cable
