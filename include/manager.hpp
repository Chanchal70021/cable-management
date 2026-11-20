#pragma once

#include "config_manager.hpp"
#include "constants.hpp"
#include "types.hpp"

#include <sdbusplus/asio/object_server.hpp>

#include <memory>

namespace cable
{

/**
 * @class Manager
 * @brief Implements the D-Bus APIs exposed by the cable-manager daemon.
 *
 * The Manager class owns the D-Bus interface for the cable-manager service.
 * All methods exposed on that interface are registered and dispatched here.
 *
 */
class Manager
{
  public:
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;

    /**
     * @brief Construct a Manager and register the D-Bus interface.
     *
     * @param[in] objectServer  sdbusplus object server
     */
    explicit Manager(sdbusplus::asio::object_server& objectServer);

    ~Manager();

  private:
    std::shared_ptr<sdbusplus::asio::dbus_interface> interface;

    /**
     * @brief Config manager instance.
     *
     * Null if initConfigMgr() failed to create it.
     */
    std::shared_ptr<ConfigMgr> configMgr;

    /**
     * @brief Create the ConfigMgr, which parses the cable configuration JSON.
     *
     * Catches any exception thrown by the ConfigMgr constructor so that a bad
     * or missing config does not abort the daemon. On failure configMgr is
     * left null.
     *
     * @note ToDo: Log a PEL on failure.
     */
    void initConfigMgr();

    /**
     * @brief Read the BMC position from the inventory path and return it
     *        to the D-Bus caller.
     *
     * @return types::BmcPosition::POSITION_0, types::BmcPosition::POSITION_1,
     *         or types::BmcPosition::INVALID_VALUE if the position cannot be
     *         determined.
     *
     * @throws sdbusplus::xyz::openbmc_project::Common::Error::ResourceNotFound
     *         if the D-Bus call fails or the position property cannot be read.
     *
     * @note   ToDo: The API may be extended to re-detect the BMC position if
     * required.
     */
    types::BmcPosition getBmcPosition();
};

} // namespace cable
