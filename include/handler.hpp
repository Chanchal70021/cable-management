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
    /**
     * @brief Construct a Handler object and initialize BMC position resolution.
     */
    Handler();

    /**
     * @brief Default destructor.
     */
    ~Handler() = default;

    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;
    Handler(Handler&&) = delete;
    Handler& operator=(Handler&&) = delete;

  private:
    /**
     * @brief Determine the BMC position, publish it under PIM, and write it to
     * file.
     */
    void readAndPublishBmcPosition();
};

} // namespace cable
