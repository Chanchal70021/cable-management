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
    auto leftCDFPPresentRes = CdfpHandler::leftCableDetected();
    if (!leftCDFPPresentRes.has_value())
    {
        errMsg =
            std::format("Failed to detect left CDFP cable with error : {}.",
                        utils::getErrorCodeMsg(leftCDFPPresentRes.error()));
        // System might be in splitmode hence proceeding further .
    }

    auto splitModeRes = utils::isSplitModeDetected();
    if (!splitModeRes.has_value())
    {
        errMsg += std::format(" Failed to detect split mode with error : {}.",
                              utils::getErrorCodeMsg(splitModeRes.error()));
    }

    if (splitModeRes.value_or(false))
    {
        // In split mode no CDFP cable should be connected , publish bmc
        // position as max_int.
        if (leftCDFPPresentRes.value_or(false))
        {
            lg2::error(
                "System is in split mode but left CDFP cable is connected");
            return;
        }

        // ToDo - splitmode case , publish Bmc position as 0
        errMsg += " System is in split mode.";
        lg2::info("{MSG}", "MSG", errMsg);
        return;
    }

    // Not in splitmode (or split mode unknown) and no left cdfp cable
    // connected
    if (!leftCDFPPresentRes.value_or(false))
    {
        errMsg += " Unable to determine BMC position.";
        lg2::error("{ERR}", "ERR", errMsg);
        // ToDo - publish Bmc position as max_int since its an error case
        // and return.
        return;
    }

    // Left Cdfp cable present : position comes from GPIO.
    auto gpioValRes = utils::readGpioVal(constants::huygensSysBmcPosGpio);
    if (!gpioValRes.has_value())
    {
        lg2::error("Failed to read GPIO '{GPIO}': {ERR}", "GPIO",
                   constants::huygensSysBmcPosGpio, "ERR",
                   utils::getErrorCodeMsg(gpioValRes.error()));
        // ToDo - publish Bmc position as max_int since its an error case
        return;
    }
    // ToDo - get bmc position and publish it under PIM .
}
} // namespace cable
