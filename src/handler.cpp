#include "handler.hpp"

namespace cable
{

types::BmcPosition Handler::readBmcPosition()
{
   // ToDo - Add the implementation of CDFP cable presence
   // and return the Bmc position via GPIO pin
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
