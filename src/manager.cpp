#include "manager.hpp"

#include "constants.hpp"

#include <phosphor-logging/lg2.hpp>

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
    // TODO: readGpioVal will be called from here with respective GPIO pins
    //       once the GPIO constants are available (added in the next commit).
    return false;
}

} // namespace cable_manager
