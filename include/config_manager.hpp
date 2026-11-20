#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace cable
{

/**
 * @class ConfigMgr
 * @brief Reads and parses the cable configuration JSON file.
 *
 * The JSON is parsed once, in the constructor, and held for the lifetime of
 * the object. Other components obtain it through getParsedJson().
 */
class ConfigMgr
{
  public:
    ConfigMgr() = delete;
    ConfigMgr(const ConfigMgr&) = delete;
    ConfigMgr& operator=(const ConfigMgr&) = delete;
    ConfigMgr(ConfigMgr&&) = delete;
    ConfigMgr& operator=(ConfigMgr&&) = delete;

    /**
     * @brief Construct a ConfigMgr and parse the given JSON file.
     *
     * @param[in] jsonPath  Path to the cable configuration JSON file.
     *
     * @throws std::runtime_error if the file cannot be opened.
     * @throws nlohmann::json::parse_error if the file is not valid JSON.
     */
    explicit ConfigMgr(const std::string& jsonPath);

    /**
     * @brief ConfigMgr Destructor.
     */
    ~ConfigMgr() = default;

    /**
     * @brief Get the parsed configuration JSON.
     *
     * @return Reference to the parsed JSON, valid for the lifetime of this
     *         object.
     */
    inline const nlohmann::json& getParsedJson() const noexcept
    {
        return parsedCableJson;
    }

  private:
    nlohmann::json parsedCableJson;
};

} // namespace cable
