#pragma once

#include <phosphor-logging/lg2.hpp>

#include <gpiod.hpp>

#include <string_view>

namespace cable_manager
{
namespace utils
{

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
inline bool readGpioVal(
    std::string_view i_gpioName,
    int& o_value) noexcept
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
        lg2::error("Exception reading GPIO line '{GPIO}': {ERR}", "GPIO",
                   i_gpioName, "ERR", l_ex.what());
        return false;
    }
}

} // namespace utils
} // namespace cable_manager
