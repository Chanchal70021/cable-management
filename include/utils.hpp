#pragma once

#include "types.hpp"

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/bus.hpp>

#include <string>

namespace utils
{

/**
 * @brief Read a D-Bus property from a given service, path, and interface.
 *
 * @param[in] service    D-Bus service name.
 * @param[in] path       D-Bus object path.
 * @param[in] interface  D-Bus interface name.
 * @param[in] property   Property name to read.
 *
 * @return Property value as types::DbusVariantType.
 *         Returns a default-constructed DbusVariantType on any failure.
 */
inline types::DbusVariantType
    readDbusProperty(const std::string& service, const std::string& path,
                     const std::string& interface, const std::string& property) noexcept
{
    types::DbusVariantType propertyValue;

    if (service.empty() || path.empty() || interface.empty() ||
        property.empty())
    {
        lg2::error(
            "Given invalid input parameters to read D-Bus property. "
            "Service: {SERVICE}, Path: {PATH}, Interface: {INTF}, Property: {PROP}",
            "SERVICE", service, "PATH", path, "INTF", interface, "PROP",
            property);
        return propertyValue;
    }

    try
    {
        auto bus = sdbusplus::bus::new_default();
        auto method =
            bus.new_method_call(service.c_str(), path.c_str(),
                                "org.freedesktop.DBus.Properties", "Get");
        method.append(interface, property);

        auto result = bus.call(method);
        result.read(propertyValue);
    }
    catch (const std::exception& e)
    {
        lg2::error(
            "Failed to read property '{PROP}' from '{PATH}' on '{SERVICE}': {ERR}",
            "PROP", property, "PATH", path, "SERVICE", service, "ERR",
            e.what());
    }

    return propertyValue;
}

} // namespace utils
