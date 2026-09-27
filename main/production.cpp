/**
 * @file production.cpp
 * @brief Simple UTC Daemon (RFC 868)
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-utcd/config/config.hpp"
#include "simple-utcd/core/server.hpp"
#include "simple-utcd/platform/windows_service.hpp"
#include "simple-utcd/utils/error_handler.hpp"
#include "simple-utcd/utils/logger.hpp"
#include "simple-utcd/utils/platform.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#ifndef SIMPLE_UTCD_VERSION
#define SIMPLE_UTCD_VERSION "1.0.0"
#endif

namespace {

std::atomic<bool> g_reload_requested{false};
std::atomic<bool> g_shutdown_requested{false};
std::string g_config_file;

void signal_handler(int sig) {
#ifdef SIGHUP
    if (sig == SIGHUP) {
        g_reload_requested = true;
        return;
    }
#endif
    if (sig == SIGINT || sig == SIGTERM) {
        g_shutdown_requested = true;
    }
}

void print_usage() {
    std::cout
        << "Usage: simple-utcd [OPTIONS] [CONFIG]\n"
        << "\n"
        << "RFC 868 time server. Serves the host clock on TCP and UDP port 37.\n"
        << "Keep the host clock synced with chrony, systemd-timesyncd, or simple-ntpd.\n"
        << "\n"
        << "Options:\n"
        << "  -c, --config FILE    Configuration file\n"
        << "  --config-test        Load and validate configuration, then exit\n"
        << "  -h, --help           Show this help\n"
        << "  -v, --version        Show version\n"
        << "  service              Windows service install, status, or uninstall\n"
        << "\n"
        << "A bare CONFIG path is accepted as well as -c.\n";
}

void print_version() {
    std::cout << "simple-utcd " << SIMPLE_UTCD_VERSION << "\n"
              << "RFC 868 time server\n"
              << "Licensed under the Apache License 2.0\n";
}

bool load_config(simple_utcd::UTCConfig& config, const std::string& path, std::string& error) {
    if (!simple_utcd::Platform::file_exists(path)) {
        error = "Configuration file not found: " + path;
        return false;
    }
    if (!config.load(path)) {
        error = "Failed to load configuration file: " + path;
        return false;
    }
    config.load_from_environment();
    if (!config.validate()) {
        error = "Configuration validation failed";
        for (const auto& item : config.get_validation_errors()) {
            error += "\n  - " + item;
        }
        return false;
    }
    return true;
}

std::string default_config_path() {
    const char* env_config = std::getenv("SIMPLE_UTCD_CONFIG");
    if (env_config && env_config[0] != '\0') {
        return env_config;
    }
    if (simple_utcd::Platform::file_exists("/etc/simple-utcd/simple-utcd.conf")) {
        return "/etc/simple-utcd/simple-utcd.conf";
    }
    if (simple_utcd::Platform::file_exists("config/simple-utcd.conf")) {
        return "config/simple-utcd.conf";
    }
    return "/etc/simple-utcd/simple-utcd.conf";
}

void request_shutdown() { g_shutdown_requested = true; }

}  // namespace

static int run_application(int argc, char* argv[]) {
    bool config_test = false;
    std::string config_file;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage();
            return 0;
        }
        if (arg == "-v" || arg == "--version") {
            print_version();
            return 0;
        }
        if (arg == "--config-test") {
            config_test = true;
            continue;
        }
        if (arg == "-c" || arg == "--config") {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << arg << " requires a file path\n";
                return 2;
            }
            config_file = argv[++i];
            continue;
        }
        if (!arg.empty() && arg[0] != '-') {
            config_file = arg;
            continue;
        }
        std::cerr << "Error: unknown option " << arg << "\n";
        print_usage();
        return 2;
    }

    if (config_file.empty()) {
        config_file = default_config_path();
    }
    g_config_file = config_file;

    try {
        simple_utcd::ErrorHandlerManager::initialize_default();
        auto logger = std::make_unique<simple_utcd::Logger>();
        auto config = std::make_unique<simple_utcd::UTCConfig>();

        std::string error;
        if (!load_config(*config, config_file, error)) {
            if (config_test) {
                std::cerr << error << "\n";
                return 1;
            }
            logger->error(error);
            return 1;
        }

        if (config_test) {
            std::cout << "Configuration is valid: " << config_file << "\n";
            return 0;
        }

        if (!config->get_log_file().empty()) {
            logger->set_log_file(config->get_log_file());
        }
        logger->enable_console(config->is_console_logging_enabled());
        logger->info("Simple UTC Daemon {} starting", std::string(SIMPLE_UTCD_VERSION));

        auto server = std::make_unique<simple_utcd::UTCServer>(config.get(), logger.get());

        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
#ifdef SIGHUP
        std::signal(SIGHUP, signal_handler);
#endif

        if (!server->start()) {
            logger->error("Failed to start UTC server");
            return 1;
        }

        logger->info("Listening on {}:{} TCP/UDP", config->get_listen_address(), config->get_listen_port());
        logger->info("Send SIGHUP to reload configuration");

        while (server->is_running() && !g_shutdown_requested.load()) {
            if (g_reload_requested.exchange(false)) {
                logger->info("Received SIGHUP, reloading configuration...");
                if (!server->reload_config(g_config_file)) {
                    logger->error("Configuration reload failed, using previous configuration");
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        logger->info("UTC Daemon shutting down...");
        server->stop();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int exit_code = 0;
    if (simple_utcd::windows_service_entry(argc, argv, exit_code,
                                           request_shutdown, run_application)) {
        return exit_code;
    }
    return run_application(argc, argv);
}
