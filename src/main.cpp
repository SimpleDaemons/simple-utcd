#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>
#include "simple_utcd/utc_server.hpp"
#include "simple_utcd/utc_config.hpp"
#include "simple_utcd/logger.hpp"
#include "simple_utcd/error_handler.hpp"

int main(int argc, char* argv[]) {
    try {
        // Initialize error handler
        simple_utcd::ErrorHandlerManager::initialize_default();

        // Initialize logger
        auto logger = std::make_unique<simple_utcd::Logger>();
        logger->info("Simple UTC Daemon starting...");

        // Load configuration
        auto config = std::make_unique<simple_utcd::UTCConfig>();
        if (!config->load("config/simple-utcd.conf")) {
            logger->error("Failed to load configuration file");
            return 1;
        }

        // Create and start UTC server
        auto server = std::make_unique<simple_utcd::UTCServer>(config.get(), logger.get());

        logger->info("UTC Daemon initialized successfully");
        logger->info("Listening on {}:{}", config->get_listen_address(), config->get_listen_port());

        // Start the server
        if (!server->start()) {
            logger->error("Failed to start UTC server");
            return 1;
        }

        // Keep the server running
        logger->info("UTC Daemon is running. Press Ctrl+C to stop.");

        // Simple signal handling for graceful shutdown
        signal(SIGINT, [](int) {
            // This will be handled by the server's stop method
            exit(0);
        });

        // Keep the main thread alive
        while (server->is_running()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
