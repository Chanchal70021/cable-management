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
    interface->register_method("GetBmcPosition",
                               [this]() {
                                   return static_cast<size_t>(getBmcPosition());
                               });

    interface->initialize();

    lg2::info("Manager initialised on path '{PATH}' interface '{IFACE}'",
              "PATH", constants::rootPath, "IFACE", constants::serviceName);
}

Manager::~Manager()
{
}

types::BmcPosition Manager::getBmcPosition()
{
    auto result = utils::readDbusProperty(
        constants::pimService, constants::systemVpdInvPath,
        constants::positionInterface, "Position");

    if (!result)
    {
        lg2::error(
            "failed to read BMC position property: {ERR}",
            "ERR", utils::getErrorCodeMsg(result.error()));
        throw sdbusplus::xyz::openbmc_project::Common::Error::ResourceNotFound();
    }

    if (const auto* pos = std::get_if<uint64_t>(&result.value()))
    {
        return static_cast<types::BmcPosition>(*pos);
    }

    lg2::error("BMC position property has unexpected type");
    throw sdbusplus::xyz::openbmc_project::Common::Error::ResourceNotFound();
}

} // namespace cable_manager
