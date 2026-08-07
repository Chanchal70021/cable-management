#pragma once

#include "constants.hpp"
#include "utils.hpp"

#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/exception.hpp>

#include <memory>

namespace cable_manager
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

    /**
     * @brief Detect whether the CDFP cable is physically present and correctly
     *        seated.
     *
     * Evaluation order: NE_CABLE_PRES_N → FE_CABLE_PRES_N → PRES_LEFT_N
     *
     * @return false/true indicates cable presence
     *
     * @throw  sdbusplus::xyz::openbmc_project::Common::Device::Error::ReadFailure
     *          in any of the following cases:
     *          • gpiod error reading NE or FE or PRES_LEFT
     *          • NE and FE values mismatch (one asserted, the other not)
     *          • NE and FE both asserted but PRES_LEFT de-asserted
     *            (cable wrongly connected / mis-seated)
     */
    bool detectCDFPCablePresence();

  private:
    std::shared_ptr<sdbusplus::asio::dbus_interface> interface;
};

} // namespace cable_manager
