#pragma once

namespace cable::constants {
// Cable Manager service
constexpr auto serviceName = "xyz.openbmc_project.Cable.Manager";
constexpr auto pimService = "xyz.openbmc_project.Inventory.Manager";

constexpr auto rootPath = "/xyz/openbmc_project/inventory/cable";
constexpr auto systemVpdInvPath = "/xyz/openbmc_project/inventory/system";

constexpr auto positionInterface =
    "xyz.openbmc_project.Inventory.Decorator.Position";

// GPIO line name for BMC position (SLED ID)
constexpr auto huygensSysBmcPosGpio = "SLED_ID";
} // namespace cable::constants
