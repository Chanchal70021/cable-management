#include "config_manager.hpp"

#include <phosphor-logging/lg2.hpp>

#include <fstream>
#include <stdexcept>

namespace cable
{

ConfigMgr::ConfigMgr(const std::string& jsonPath)
{
    std::ifstream jsonFile(jsonPath);
    if (!jsonFile.is_open())
    {
        throw std::runtime_error(
            "Failed to open config JSON file: " + jsonPath);
    }

    try
    {
        parsedCableJson = nlohmann::json::parse(jsonFile);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        throw;
    }
}

} // namespace cable
