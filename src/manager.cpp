#include "manager.hpp"

#include "constants.hpp"
#include "utils.hpp"

#include <phosphor-logging/lg2.hpp>
#include <xyz/openbmc_project/Common/Device/error.hpp>

#include <string_view>
#include <vector>

namespace cable_manager
{

Manager::Manager(sdbusplus::asio::object_server& objectServer) :
    interface(objectServer.add_interface(constants::rootPath,
                                             constants::serviceName))
{
    interface->register_method("DetectCDFPCablePresence",
                               [this]() { return detectCDFPCablePresence(); });

    interface->initialize();

    lg2::info("Manager initialised on path '{PATH}' interface '{IFACE}'",
              "PATH", constants::rootPath, "IFACE", constants::serviceName);
}

Manager::~Manager()
{
}

bool Manager::detectCDFPCablePresence()
{
    int l_neRaw = 0;
    if (!utils::readGpioVal(constants::gpioNearEndPresN, l_neRaw))
    {
        // TODO: define proper error files for cable-manager in future.
        throw sdbusplus::xyz::openbmc_project::Common::Device::Error::
            ReadFailure();
    }

    int l_feRaw = 0;
    if (!utils::readGpioVal(constants::gpioFarEndPresN, l_feRaw))
    {
        // TODO: define proper error files for cable-manager in future.
        throw sdbusplus::xyz::openbmc_project::Common::Device::Error::
            ReadFailure();
    }

    const bool l_nePresent = (l_neRaw == 0);
    const bool l_fePresent = (l_feRaw == 0);

    if (!l_nePresent && !l_fePresent)
    {
        lg2::info("Near-end and far-end both absent – cable not present NE_CABLE_PRES_N={NE}, "
                  "FE_CABLE_PRES_N={FE}",
                  "NE", l_neRaw, "FE", l_feRaw);
        return false;
    }

    if (l_nePresent != l_fePresent)
    {
        lg2::error("Cable presence pin mismatch: NE_CABLE_PRES_N={NE}, "
                   "FE_CABLE_PRES_N={FE}",
                   "NE", l_neRaw, "FE", l_feRaw);
        // TODO: define proper error files for cable-manager in future.
        throw sdbusplus::xyz::openbmc_project::Common::Device::Error::
            ReadFailure();
    }

    int l_leftRaw = 0;
    if (!utils::readGpioVal(constants::gpioPresLeftN, l_leftRaw))
    {
        // TODO: define proper error files for cable-manager in future.
        throw sdbusplus::xyz::openbmc_project::Common::Device::Error::
            ReadFailure();
    }

    // Active-low: raw 0 → cable correctly seated.
    // raw 1 (de-asserted) → cable wrongly connected / mis-seated.
    if (l_leftRaw != 0)
    {
        lg2::error("Cable wrongly connected: NearEnd and FarEnd both present but "
                   "left present cable pin is de-asserted (raw={VAL}) – check cable "
                   "orientation/seating",
                   "VAL", l_leftRaw);
        // TODO: define proper error files for cable-manager in future.
        throw sdbusplus::xyz::openbmc_project::Common::Device::Error::
            ReadFailure();
    }

    lg2::info("Cable is present");
    return true;
}

} // namespace cable_manager
