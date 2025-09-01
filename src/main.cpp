#include <iostream>
#include <memory>
#include "simple_utcd/utc_server.hpp"
#include "simple_utcd/utc_config.hpp"
#include "simple_utcd/logger.hpp"

int main(int argc, char* argv[]) {
    try {
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
        
        // Start the server (this will block)
        server->start();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
