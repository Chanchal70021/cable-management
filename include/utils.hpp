#pragma once

#include "constants.hpp"
#include "error_code.hpp"
#include "types.hpp"

#include <gpiod.hpp>
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

/**
 * @brief Read a single GPIO line
 *
 * Reads the value of a specified GPIO line. The caller is responsible for
 * interpreting the returned value based on their specific use case.
 *
 * @param[in]  i_gpioName  Name of the GPIO line to read.
 * @param[out] o_value     Output parameter for the GPIO value read
 *
 * @return true on success, false on error/failure
 *
 */
inline bool readGpioVal(const std::string& i_gpioName, int& o_value) noexcept
{
    try
    {
        auto l_line = gpiod::find_line(std::string(i_gpioName));

        if (!l_line)
        {
            lg2::error("Failed to find GPIO line: {G}", "G", i_gpioName);
            return false;
        }

        l_line.request(
            {"cable-manager", gpiod::line_request::DIRECTION_INPUT, 0});

        o_value = l_line.get_value();

        // Explicitly releasing the line resource
        l_line.release();
        return true;
    }
    catch (const std::exception& l_ex)
    {
        lg2::error("Exception reading GPIO lines: {ERR}", "ERR", l_ex.what());
        return false;
    }
}

/**
 * @brief Detect whether the CDFP cable is physically present and correctly
 *        plugged.
 *
 * Evaluation order: NE_CABLE_PRES_N → FE_CABLE_PRES_N → PRES_LEFT_N
 *
 * @param[out] o_cableCheckSucceeded  Set to false if a GPIO read error or
 *             pin mismatch/mis-plug prevented a definitive result; true
 *             otherwise (even when cable is cleanly absent).
 *
 * @return true if the cable is present and correctly seated, false
 *         otherwise.
 */
inline bool detectLeftCDFPCablePresence(bool& o_cableCheckSucceeded) noexcept
{
    // ToDo - This API needs to be extended in future to detect right CDFP cable
    // presence as well as log PEL in any error scenario .

    o_cableCheckSucceeded = true;

    int l_neRaw = 0;
    if (!readGpioVal(cable::constants::gpioNearEndPresN, l_neRaw))
    {
        lg2::error("GPIO read failed for NE_CABLE_PRES_N");
        o_cableCheckSucceeded = false;
        return false;
    }

    int l_feRaw = 0;
    if (!readGpioVal(cable::constants::gpioFarEndPresN, l_feRaw))
    {
        lg2::error("GPIO read failed for FE_CABLE_PRES_N");
        o_cableCheckSucceeded = false;
        return false;
    }

    const bool l_nePresent = (l_neRaw == 0);
    const bool l_fePresent = (l_feRaw == 0);

    if (!l_nePresent && !l_fePresent)
    {
        lg2::info("Near-end and far-end both absent – cable not present "
                  "NE_CABLE_PRES_N={NE}, FE_CABLE_PRES_N={FE}",
                  "NE", l_neRaw, "FE", l_feRaw);
        return false;
    }

    if (l_nePresent != l_fePresent)
    {
        lg2::error("Cable presence pin mismatch: NE_CABLE_PRES_N={NE}, "
                   "FE_CABLE_PRES_N={FE}",
                   "NE", l_neRaw, "FE", l_feRaw);
        o_cableCheckSucceeded = false;
        return false;
    }

    int l_leftRaw = 0;
    if (!readGpioVal(cable::constants::gpioPresLeftN, l_leftRaw))
    {
        lg2::error("GPIO read failed for PRES_LEFT_N");
        o_cableCheckSucceeded = false;
        return false;
    }

    // Active-low: raw 0 → cable correctly seated.
    // raw 1 (de-asserted) → cable wrongly connected / mis-seated.
    if (l_leftRaw != 0)
    {
        lg2::error("Cable wrongly connected: NearEnd and FarEnd both present "
                   "but left present pin de-asserted (raw={VAL})",
                   "VAL", l_leftRaw);
        o_cableCheckSucceeded = false;
        return false;
    }

    return true;
}
} // namespace cable::utils
