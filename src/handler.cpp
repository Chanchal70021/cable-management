#include "handler.hpp"
#include "utils.hpp"

namespace cable
{

types::BmcPosition Handler::readBmcPosition()
{
   // ToDo - Add the implementation of Bmc position via GPIO pin
    bool l_cableCheckSucceeded = true;
    [[maybe_unused]] const bool l_cablePresent =
        utils::detectLeftCDFPCablePresence(l_cableCheckSucceeded);
    return types::BmcPosition::INVALID_VALUE;
}

Handler::Handler()
{
    setBMCPosition();
}

void Handler::setBMCPosition()
{
    // ToDo - Add the implementation of get the Bmc position
    // of a specific system via IM value, publish it and
    // write the Bmc position to file.

    readBmcPosition();

}
} // namespace cable
