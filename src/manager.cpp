#include "config.h"

#include "manager.hpp"

#include "constants.hpp"
#include "utils.hpp"

#include <phosphor-logging/lg2.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

#include <string>
#include <vector>

namespace cable
{

Manager::Manager(sdbusplus::asio::object_server& objectServer) :
    interface(
        objectServer.add_interface(constants::rootPath, constants::serviceName))
{
    initConfigMgr();

    interface->register_method("GetBmcPosition", [this]() {
        return static_cast<size_t>(getBmcPosition());
    });

    interface->register_method("ValidateCdfpCables",
                               [](const std::string& /*scope*/) {
                                   // TODO: This is where the cdfp1CableDetected
                                   // API will be consumed. This is where config
                                   // manager class's getParsedJson API will be
                                   // consumed.
                                   return std::vector<std::string>{};
                               });

    interface->initialize();

    lg2::info("Manager initialised on path '{PATH}' interface '{IFACE}'",
              "PATH", constants::rootPath, "IFACE", constants::serviceName);
}

Manager::~Manager() {}

void Manager::initConfigMgr()
{
    try
    {
        // TODO: For now the config is hardcoded to the Huygens 4N JSON only.
        // Revisit once multiple JSON files are involved, so the file to parse
        // is selected based on the system configuration.
        configMgr = std::make_shared<ConfigMgr>(HUYGENS_4N_CABLE_JSON_PATH);
    }
    catch (const std::exception& e)
    {
        // TODO: Log a PEL for the config failure.
        lg2::error("Failed to initialise config manager: {ERROR}", "ERROR", e);
    }
}

types::BmcPosition Manager::getBmcPosition()
{
    auto result = utils::readDbusProperty(
        constants::pimService, constants::systemVpdInvPath,
        constants::positionInterface, "Position");

    if (!result)
    {
        lg2::error("failed to read BMC position property: {ERR}", "ERR",
                   utils::getErrorCodeMsg(result.error()));
        throw sdbusplus::xyz::openbmc_project::Common::Error::
            ResourceNotFound();
    }

    if (const auto* pos = std::get_if<uint64_t>(&result.value()))
    {
        return static_cast<types::BmcPosition>(*pos);
    }

    lg2::error("BMC position property has unexpected type");
    throw sdbusplus::xyz::openbmc_project::Common::Error::ResourceNotFound();
}

} // namespace cable
