#include "handler.hpp"

#include "utils.hpp"

namespace cable
{

Handler::Handler()
{
    // TODO to determine if the system qualifies for BMC position.
    readAndPublishBmcPosition();
}

void Handler::readAndPublishBmcPosition()
{
    auto leftCDFPPresentRes = utils::cdfpLeftCableDetected();
    if (!leftCDFPPresentRes.has_value())
    {
        lg2::error("Failed to detect left CDFP cable: {ERR}", "ERR",
                   utils::getErrorCodeMsg(leftCDFPPresentRes.error()));
        // ToDo - publish Bmc position as max_int since its an error case and
        // return.
        return;
    }

    // ToDo: based on configuration read right CDFP cable

    auto splitModeRes = utils::isSplitModeDetected();
    if (!splitModeRes.has_value())
    {
        // ToDo - Log error. Don't return: if the left CDFP cable is present
        // the position can still be read from GPIO below.
    }

    if (splitModeRes.value_or(false))
    {
        // In split mode no CDFP cable should be connected.
        if (leftCDFPPresentRes.value())
        {
            // ToDo - publish Bmc position as max_int since its an error case
            // and return.
            return;
        }

        // ToDo - splitmode case , publish Bmc position as 0
        return;
    }

    // Not in splitmode (or split mode unknown) and no left cdfp cable
    // connected
    if (!leftCDFPPresentRes.value())
    {
        // ToDo - publish Bmc position as max_int , left Cdfp cable is absent
        // and system is not in splitmode (or split mode detection failed).
        return;
    }

    // Left Cdfp cable present (with or without right): position comes from
    // GPIO.
    auto gpioValRes = utils::readGpioVal(constants::huygensSysBmcPosGpio);
    if (!gpioValRes.has_value())
    {
        // ToDo - Log error and publish Bmc position as max_int since its an
        // error case
        return;
    }
    // ToDo - get bmc position and publish it under PIM .
}
} // namespace cable
