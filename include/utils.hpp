#pragma once

#include "error_code.hpp"
#include "types.hpp"

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/bus.hpp>

#include <expected>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cable::utils
{

/**
 * @brief Returns a human-readable message for given ErrorCode.
 *
 * @param[in] code  The error code to look up.
 *
 * @return A string_view describing the error, or "unknown error" if the
 *         code is not present in the table.
 */
inline std::string_view
    getErrorCodeMsg(const ErrorCode& code) noexcept
{
    static const std::unordered_map<ErrorCode, std::string_view>
        errorCodeMessages{
            {ErrorCode::INVALID_INPUT,      "invalid input parameters"},
            {ErrorCode::STANDARD_EXCEPTION, "standard exception caught"},
        };

    if (const auto it = errorCodeMessages.find(code);
        it != errorCodeMessages.end())
    {
        return it->second;
    }

    return "unknown error";
}

/**
 * @brief Read a D-Bus property from a given service, path, and interface.
 *
 * @param[in] service    D-Bus service name.
 * @param[in] path       D-Bus object path.
 * @param[in] interface  D-Bus interface name.
 * @param[in] property   Property name to read.
 *
 * @return The property value as types::DbusVariantType on success, or a
 *         cable::ErrorCode on failure:
 *           - ErrorCode::INVALID_INPUT      – one or more input strings are empty.
 *           - ErrorCode::STANDARD_EXCEPTION – the D-Bus call threw an exception.
 */
inline std::expected<types::DbusVariantType, ErrorCode>
    readDbusProperty(const std::string& service, const std::string& path,
                     const std::string& interface,
                     const std::string& property) noexcept
{
    if (service.empty() || path.empty() || interface.empty() ||
        property.empty())
    {
        lg2::error(
            "Given invalid input parameters to read D-Bus property. "
            "Service: {SERVICE}, Path: {PATH}, Interface: {INTF}, Property: {PROP}",
            "SERVICE", service, "PATH", path, "INTF", interface, "PROP",
            property);
        return std::unexpected(ErrorCode::INVALID_INPUT);
    }

    try
    {
        auto bus = sdbusplus::bus::new_default();
        auto method =
            bus.new_method_call(service.c_str(), path.c_str(),
                                "org.freedesktop.DBus.Properties", "Get");
        method.append(interface, property);

        types::DbusVariantType propertyValue;
        auto result = bus.call(method);
        result.read(propertyValue);
        return propertyValue;
    }
    catch (const std::exception& e)
    {
        lg2::error(
            "Failed to read property '{PROP}' from '{PATH}' on '{SERVICE}': {ERR}",
            "PROP", property, "PATH", path, "SERVICE", service, "ERR",
            e.what());
        return std::unexpected(ErrorCode::STANDARD_EXCEPTION);
    }
}

} // namespace cable::utils
