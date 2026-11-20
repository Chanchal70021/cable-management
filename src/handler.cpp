#include "handler.hpp"

#include "cdfp_handler.hpp"
#include "utils.hpp"

#include <phosphor-logging/lg2.hpp>

#include <format>

namespace cable
{

Handler::Handler()
{
    // TODO to determine if the system qualifies for BMC position.
    readAndPublishBmcPosition();
}

void Handler::readAndPublishBmcPosition()
{
    std::string errMsg = "";
    auto cdfp0PresentRes = CdfpHandler::cdfp0CableDetected();
    if (!cdfp0PresentRes.has_value())
    {
        errMsg = std::format("Failed to detect CDFP cable 0 with error : {}.",
                             utils::getErrorCodeMsg(cdfp0PresentRes.error()));
        // System might be in splitmode hence proceeding further .
    }

    auto splitModeRes = utils::isSplitModeDetected();
    if (!splitModeRes.has_value())
    {
        errMsg += std::format(" Failed to detect split mode with error : {}.",
                              utils::getErrorCodeMsg(splitModeRes.error()));
    }

    const types::BmcPosition position = [&]() -> types::BmcPosition {
        if (splitModeRes.value_or(false))
        {
            // In split mode no CDFP cable should be connected.
            if (cdfp0PresentRes.value_or(false))
            {
                lg2::error(
                    "System is in split mode but CDFP cable 0 is connected");
                return types::BmcPosition::INVALID_VALUE;
            }

            errMsg += " System is in split mode.";
            lg2::info("{MSG}", "MSG", errMsg);
            return types::BmcPosition::POSITION_0;
        }

        // Not in splitmode (or split mode unknown) and no CDFP cable 0
        // connected
        if (!cdfp0PresentRes.value_or(false))
        {
            errMsg += " Unable to determine BMC position.";
            lg2::error("{ERR}", "ERR", errMsg);
            return types::BmcPosition::INVALID_VALUE;
        }

        // CDFP cable 0 present : position comes from GPIO.
        auto gpioValRes = utils::readGpioVal(constants::huygensSysBmcPosGpio);
        if (!gpioValRes.has_value())
        {
            lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                       constants::huygensSysBmcPosGpio, "ERR",
                       utils::getErrorCodeMsg(gpioValRes.error()));
            return types::BmcPosition::INVALID_VALUE;
        }
        return static_cast<types::BmcPosition>(gpioValRes.value());
    }();

    lg2::info("Evaluated Bmc position: {POS}", "POS",
              static_cast<size_t>(position));
    // ToDo - publish the position under PIM.
}
} // namespace cable
