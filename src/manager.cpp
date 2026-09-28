#include "manager.hpp"

#include "constants.hpp"
#include "utils.hpp"

#include <phosphor-logging/lg2.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

namespace cable_manager
{

Manager::Manager(sdbusplus::asio::object_server& objectServer) :
    interface(objectServer.add_interface(constants::rootPath,
                                         constants::serviceName))
{
    interface->register_method(constants::getBMCPositionMethod,
                               [this]() {
                                   return static_cast<size_t>(getBMCPosition());
                               });

    interface->initialize();

    lg2::info("Manager initialised on path '{PATH}' interface '{IFACE}'",
              "PATH", constants::rootPath, "IFACE", constants::serviceName);
}

Manager::~Manager()
{
}

types::BmcPosition Manager::getBMCPosition()
{
    auto val = utils::readDbusProperty(
        constants::pimService, constants::systemVpdInvPath,
        constants::positionInterface, constants::positionPropertyName);

    if (const auto* pos = std::get_if<uint64_t>(&val))
    {
        return static_cast<types::BmcPosition>(*pos);
    }

    lg2::error("cable-manager: failed to get a valid BMC position value");
    throw sdbusplus::xyz::openbmc_project::Common::Error::ResourceNotFound();
}

} // namespace cable_manager
