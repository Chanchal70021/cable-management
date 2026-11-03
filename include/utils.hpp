#pragma once

#include "constants.hpp"
#include "error_code.hpp"
#include "types.hpp"

#include <sys/wait.h>

#include <gpiod.hpp>
#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/bus.hpp>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <expected>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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
inline std::string_view getErrorCodeMsg(const ErrorCode& code) noexcept
{
    static const std::unordered_map<ErrorCode, std::string_view>
        errorCodeMessages{
            {ErrorCode::INVALID_INPUT, "invalid input parameter"},
            {ErrorCode::STANDARD_EXCEPTION, "standard exception caught"},
            {ErrorCode::GPIO_LINE_NOT_FOUND, "GPIO line not found"},
            {ErrorCode::GPIO_READ_MISMATCH,
             "near-end and far-end cable presence GPIO pin value mismatch"},
            {ErrorCode::CABLE_WRONGLY_CONNECTED, "cable wrongly connected"},
            {ErrorCode::FAR_END_FAILED, "downlink detection failed"},
            {ErrorCode::NEAR_END_FAILED, "uplink detection failed"},
            {ErrorCode::CABLE_STATUS_UNKNOWN, "failed to detect cable status"},
            {ErrorCode::SYS_CMD_FAILED, "system command failed"},
            {ErrorCode::FIELD_MODE_READ_FAILED,
             "failed to read or parse fieldmode u-boot variable"},
            {ErrorCode::VPD_MODE_READ_FAILED,
             "failed to read or parse vpdmode u-boot variable"},
            {ErrorCode::UBOOT_VARIABLE_NOT_FOUND, "u-boot variable not found"}};

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
 *           - ErrorCode::INVALID_INPUT      – one or more input strings are
 * empty.
 *           - ErrorCode::STANDARD_EXCEPTION – the D-Bus call threw an
 * exception.
 */
inline std::expected<types::DbusVariantType, ErrorCode> readDbusProperty(
    const std::string& service, const std::string& path,
    const std::string& interface, const std::string& property) noexcept
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
 * @brief Read a single GPIO line.
 *
 * Reads the value of a specified GPIO line. The caller is responsible for
 * interpreting the returned value based on their specific use case.
 *
 * @param[in] gpioName  Name of the GPIO line to read.
 *
 * @return GPIO value (0/1) on success, or ErrorCode on failure
 *
 */
inline std::expected<int, ErrorCode> readGpioVal(
    const std::string& gpioName) noexcept
{
    if (gpioName.empty())
    {
        return std::unexpected(ErrorCode::INVALID_INPUT);
    }

    try
    {
        auto line = gpiod::find_line(gpioName);

        if (!line)
        {
            return std::unexpected(ErrorCode::GPIO_LINE_NOT_FOUND);
        }

        line.request(
            {"cable-manager", gpiod::line_request::DIRECTION_INPUT, 0});

        const int value = line.get_value();

        // Explicitly releasing the line resource
        line.release();
        return value;
    }
    catch (const std::exception& ex)
    {
        lg2::error("Failed to read GPIO line '{GPIO}': {ERR}", "GPIO", gpioName,
                   "ERR", ex.what());
        return std::unexpected(ErrorCode::STANDARD_EXCEPTION);
    }
}

/**
 * @brief Convert a string to lower case in place.
 *
 * @param[in,out] string - String to convert.
 */
inline void toLower(std::string& string) noexcept
{
    std::transform(string.begin(), string.end(), string.begin(),
                   [](unsigned char ch) { return std::tolower(ch); });
}

/**
 * @brief Execute a shell command and capture output
 *
 * Executes a shell command using popen and captures the output lines.
 * The exit status of the command is checked via pclose; a non-zero exit
 * status is treated as a failure even if output was produced.
 *
 * @param[in] command - Command to execute
 *
 * @return std::expected<std::vector<std::string>, ErrorCode> - On
 * success, contains the captured output lines. On failure, contains
 * ErrorCode::UBOOT_VARIABLE_NOT_FOUND if a fw_printenv command exits with
 * status 1, ErrorCode::SYS_CMD_FAILED if popen fails or the command exits
 * with any other non-zero status, or ErrorCode::STANDARD_EXCEPTION on an
 * unexpected exception.
 */
inline std::expected<std::vector<std::string>, ErrorCode> executeCmd(
    const std::string& command) noexcept
{
    try
    {
        using pCloseFunc = int (*)(FILE*);
        std::unique_ptr<FILE, pCloseFunc> pipe(popen(command.c_str(), "r"),
                                               pclose);

        if (!pipe)
        {
            int err = errno;
            lg2::error("popen failed for command [{C}], errno: {E} ({MSG})",
                       "C", command, "E", err, "MSG", strerror(err));
            return std::unexpected(ErrorCode::SYS_CMD_FAILED);
        }

        std::vector<std::string> output;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
        {
            output.push_back(std::string(buffer));
        }

        // Release ownership so we can call pclose manually and inspect the
        // command's exit status — the destructor would discard it silently.
        int status = pclose(pipe.release());
        int err = errno;

        if (status == -1)
        {
            lg2::error("pclose failed for command [{C}], errno: {E} ({MSG})",
                       "C", command, "E", err, "MSG", strerror(err));
            return std::unexpected(ErrorCode::SYS_CMD_FAILED);
        }

        if (!WIFEXITED(status))
        {
            lg2::error("Command [{C}] did not exit normally, wait status: {S}",
                       "C", command, "S", status);
            return std::unexpected(ErrorCode::SYS_CMD_FAILED);
        }

        if (WEXITSTATUS(status) != 0)
        {
            // ONLY treat exit status 1 as "Variable Not Found" if the command
            // is actually fw_printenv
            if (WEXITSTATUS(status) == 1 &&
                command.find("fw_printenv") != std::string::npos)
            {
                // Not logged here: fw_printenv already reports the undefined
                // variable on its own, so logging again would be redundant.
                return std::unexpected(ErrorCode::UBOOT_VARIABLE_NOT_FOUND);
            }

            lg2::error("Command [{C}] exited with non-zero status: {S}", "C",
                       command, "S", WEXITSTATUS(status));
            return std::unexpected(ErrorCode::SYS_CMD_FAILED);
        }
        return output;
    }
    catch (const std::exception& ex)
    {
        lg2::error("Failed to execute system command '{CMD}': {ERR}", "CMD",
                   command, "ERR", ex.what());
        return std::unexpected(ErrorCode::STANDARD_EXCEPTION);
    }
}

/**
 * @brief API to check if field mode is enabled
 *
 * Checks the U-boot fieldmode variable to determine if field mode is enabled.
 *
 * @return std::expected<bool, ErrorCode> - On success, contains
 * true if field mode is enabled, false otherwise. On failure, contains
 * ErrorCode::FIELD_MODE_READ_FAILED.
 */
inline std::expected<bool, ErrorCode> isFieldModeEnabled() noexcept
{
    auto cmdOutput = executeCmd("/sbin/fw_printenv fieldmode");

    if (!cmdOutput)
    {
        return std::unexpected(ErrorCode::FIELD_MODE_READ_FAILED);
    }

    if (!cmdOutput->empty())
    {
        toLower(cmdOutput->front());

        // Remove the new line character from the string
        if (cmdOutput->front().ends_with('\n'))
        {
            cmdOutput->front().pop_back();
        }

        return cmdOutput->front() == "fieldmode=true";
    }

    return false;
}

/**
 * @brief API to get the VPD collection mode.
 *
 * VPD collection mode can be hardware, mixed mode or file mode. This is
 * determined by reading a u-boot variable.
 *
 * @return std::expected<types::VpdCollectionMode, ErrorCode> - On success,
 * contains the VPD collection mode. On failure, contains
 * ErrorCode::VPD_MODE_READ_FAILED .
 */
inline std::expected<types::VpdCollectionMode, ErrorCode>
    getVpdCollectionMode() noexcept
{
    auto cmdOutput = executeCmd("/sbin/fw_printenv vpdmode");

    if (!cmdOutput)
    {
        return std::unexpected(ErrorCode::VPD_MODE_READ_FAILED);
    }

    if (!cmdOutput->empty())
    {
        toLower(cmdOutput->front());

        // Remove the new line character from the string
        if (cmdOutput->front().ends_with('\n'))
        {
            cmdOutput->front().pop_back();
        }

        if (cmdOutput->front() == "vpdmode=hardware")
        {
            return types::VpdCollectionMode::HARDWARE_MODE;
        }
        else if (cmdOutput->front() == "vpdmode=mixed")
        {
            return types::VpdCollectionMode::MIXED_MODE;
        }
        else if (cmdOutput->front() == "vpdmode=file")
        {
            return types::VpdCollectionMode::FILE_MODE;
        }
    }

    return types::VpdCollectionMode::DEFAULT_MODE;
}

/**
 * @brief Detect whether the system is configured in split mode.
 *
 * @return true if split mode is detected, false if not, or
 *         ErrorCode on failure.
 */
inline std::expected<bool, ErrorCode> isSplitModeDetected() noexcept
{
    auto fieldModeRes = isFieldModeEnabled();
    if (!fieldModeRes)
    {
        return std::unexpected(fieldModeRes.error());
    }

    if (fieldModeRes.value())
    {
        // Field mode set: not split mode, VPD mode is not needed.
        return false;
    }

    auto vpdModeRes = getVpdCollectionMode();
    if (!vpdModeRes)
    {
        return std::unexpected(vpdModeRes.error());
    }

    return vpdModeRes.value() == types::VpdCollectionMode::FILE_MODE;
}
} // namespace cable::utils
