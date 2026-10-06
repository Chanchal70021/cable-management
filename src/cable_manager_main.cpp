#include "constants.hpp"
#include "handler.hpp"
#include "manager.hpp"

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <boost/asio/io_context.hpp>

#include <cstdlib>
#include <memory>

int main()
{
    try
    {
        boost::asio::io_context io_con;

        auto bus = std::make_shared<sdbusplus::asio::connection>(io_con);

        bus->request_name(cable::constants::serviceName);

        sdbusplus::asio::object_server objectServer(bus);
        objectServer.add_manager(cable::constants::rootPath);

        auto manager = std::make_shared<cable::Manager>(objectServer);

        auto handler = std::make_shared<cable::Handler>();

        lg2::info("Cable Manager daemon initialised, D-Bus service "
                  "'{SERVICE}' registered",
                  "SERVICE", cable::constants::serviceName);

        io_con.run();

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        lg2::error("Cable Manager failed: {ERROR}", "ERROR", e);
        return EXIT_FAILURE;
    }
}
