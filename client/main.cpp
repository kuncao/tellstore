#include "Client.hpp"
#include "ClientConfig.hpp"

#include <util/Epoch.hpp>
#include <util/Logging.hpp>

#include <crossbow/infinio/EventDispatcher.hpp>
#include <crossbow/program_options.hpp>

#include <iostream>

int main(int argc, const char** argv) {
    tell::store::ClientConfig clientConfig;
    bool help = false;
    crossbow::string logLevel("DEBUG");

    auto opts = crossbow::program_options::create_options(argv[0],
            crossbow::program_options::value<'h'>("help", &help),
            crossbow::program_options::value<'l'>("log-level", &logLevel),
            crossbow::program_options::value<'s'>("server", &clientConfig.server),
            crossbow::program_options::value<'p'>("port", &clientConfig.port));

    try {
        crossbow::program_options::parse(opts, argc, argv);
    } catch (crossbow::program_options::argument_not_found e) {
        std::cerr << e.what() << std::endl << std::endl;
        crossbow::program_options::print_help(std::cout, opts);
        return 1;
    }

    if (help) {
        crossbow::program_options::print_help(std::cout, opts);
        return 0;
    }

    clientConfig.infinibandLimits.receiveBufferCount = 128;
    clientConfig.infinibandLimits.sendBufferCount = 128;
    clientConfig.infinibandLimits.bufferLength = 1024;
    clientConfig.infinibandLimits.sendQueueLength = 128;

    tell::store::logger->config.level = tell::store::logLevelFromString(logLevel);

    LOG_INFO("Starting TellStore client");
    tell::store::init();

    // Initialize network stack
    crossbow::infinio::EventDispatcher dispatcher(clientConfig.networkThreads);
    tell::store::Client client(dispatcher, clientConfig);
    client.init();

    LOG_INFO("Start dispatcher threads");
    dispatcher.run();

    LOG_INFO("Exiting TellStore client");
    return 0;
}