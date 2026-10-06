#pragma once
#include "types.hpp"


namespace cable
{

/**
 * @class Handler
 * @brief Handles platform-level startup logic for the cable-manager daemon.
 *
 * Responsible for detecting the system type, resolving the BMC position,
 * and publishing it. Further functionalities can be added here as new
 * requirements arise. Instantiated once at daemon startup before the
 * D-Bus event loop begins.
 */
class Handler
{
  public:
    Handler();
    ~Handler() = default;

    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;
    Handler(Handler&&) = delete;
    Handler& operator=(Handler&&) = delete;

  private:
    /**
     * @brief Determine the Bmc position and publish it.
     *
     * Resolves the Bmc position, then publishes it and
     * persists it to file.
     */
    void setBMCPosition();

    /**
     * @brief Determine the BMC position.
     *
     * Uses detectLeftCDFPCablePresence() to check cable presence. If absent,
     * applies the fallback logic. If present, reads the SLED_ID GPIO.
     *
     * @return Resolved BMC position, or types::BmcPosition::INVALID_VALUE on
     *         any unrecoverable failure path.
     */
    types::BmcPosition readBmcPosition();
};

} // namespace cable
