#pragma once

namespace cable_manager
{
namespace constants
{
constexpr auto serviceName = "xyz.openbmc_project.Cable.Manager";
constexpr auto rootPath = "/xyz/openbmc_project/inventory/cable";

// GPIO line names for CDFP0 cable presence detection
constexpr auto gpioFarEndPresN  = "CDFP0_FE_CABLE_PRES_N";
constexpr auto gpioNearEndPresN = "CDFP0_NE_CABLE_PRES_N";
constexpr auto gpioPresLeftN    = "CDFP0_PRES_LEFT_N";

} // namespace constants
} // namespace cable_manager
