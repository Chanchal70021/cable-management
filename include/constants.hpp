#pragma once

namespace constants {
// Cable Manager service
constexpr auto serviceName = "xyz.openbmc_project.Cable.Manager";
constexpr auto pimService = "xyz.openbmc_project.Inventory.Manager";

constexpr auto rootPath = "/xyz/openbmc_project/inventory/cable";
constexpr auto systemVpdInvPath = "/xyz/openbmc_project/inventory/system";

constexpr auto getBMCPositionMethod = "GetBMCPosition";

constexpr auto positionInterface =
    "xyz.openbmc_project.Inventory.Decorator.Position";
constexpr auto positionPropertyName = "Position";
} // namespace constants
