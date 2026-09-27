/*
 * src/core/utc_server.cpp
 *
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "simple-utcd/core/server.hpp"
#include "simple-utcd/core/connection.hpp"
#include "simple-utcd/core/packet.hpp"
#include "simple-utcd/utils/platform.hpp"
#include "simple-utcd/utils/error_handler.hpp"
#include "simple-utcd/utils/metrics.hpp"
#include "simple-utcd/utils/health_check.hpp"
#include "simple-utcd/network/async_io.hpp"
#include "simple-utcd/security/rate_limiter.hpp"
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace simple_utcd {
namespace {

bool set_reuseaddr(int fd) {
    int reuse = 1;
    return Platform::set_socket_option(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
}

std::string format_address(const sockaddr_storage& address) {
    char text[INET6_ADDRSTRLEN] = {};
    if (address.ss_family == AF_INET) {
        const auto* ipv4 = reinterpret_cast<const sockaddr_in*>(&address);
        if (inet_ntop(AF_INET, &ipv4->sin_addr, text, sizeof(text)) == nullptr) {
            return "unknown";
        }
        return text;
    }
    if (address.ss_family == AF_INET6) {
        const auto* ipv6 = reinterpret_cast<const sockaddr_in6*>(&address);
        if (inet_ntop(AF_INET6, &ipv6->sin6_addr, text, sizeof(text)) == nullptr) {
            return "unknown";
        }
        return text;
    }
    return "unknown";
}

bool wait_readable(int fd) {
#ifdef _WIN32
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    return select(0, &read_fds, nullptr, nullptr, &timeout) > 0;
#else
    pollfd ready{};
    ready.fd = fd;
    ready.events = POLLIN;
    return poll(&ready, 1, 200) > 0 && (ready.revents & POLLIN) != 0;
#endif
}

bool bind_ipv6(int fd, const std::string& address, int port) {
    int v6only = 1;
    Platform::set_socket_option(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

    sockaddr_in6 addr{};
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(static_cast<uint16_t>(port));
    const std::string bind_address =
        (address.empty() || address == "0.0.0.0" || address == "::") ? "::" : address;
    if (inet_pton(AF_INET6, bind_address.c_str(), &addr.sin6_addr) != 1) {
        return false;
    }
    return ::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
}

}  // namespace

UTCServer::UTCServer(UTCConfig* config, Logger* logger)
    : config_(config)
    , logger_(logger)
    , running_(false)
    , active_connections_(0)
    , total_connections_(0)
    , packets_sent_(0)
    , packets_received_(0)
    , rate_limiter_(std::make_unique<RateLimiter>())
    , performance_metrics_(std::make_unique<PerformanceMetrics>())
    , health_checker_(std::make_unique<HealthChecker>())
    , async_io_manager_(std::make_unique<AsyncIOManager>(config ? config->get_worker_threads() : 4))
{
    if (config_ && rate_limiter_) {
        rate_limiter_->set_enabled(config_->is_rate_limit_enabled());
        if (config_->get_rate_limit_rps() > 0) {
            rate_limiter_->set_rate(static_cast<uint64_t>(config_->get_rate_limit_rps()));
            rate_limiter_->set_global_rate(static_cast<uint64_t>(config_->get_rate_limit_rps()) * 32);
        }
        if (config_->get_rate_limit_burst() > 0) {
            rate_limiter_->set_burst_size(static_cast<uint64_t>(config_->get_rate_limit_burst()));
            rate_limiter_->set_global_burst(static_cast<uint64_t>(config_->get_rate_limit_burst()) * 32);
        }
    }

    if (logger_) {
        logger_->info("UTC Server initialized");
    }

    if (async_io_manager_) {
        async_io_manager_->start();
    }
}

UTCServer::~UTCServer() {
    if (async_io_manager_) {
        async_io_manager_->stop();
    }
    stop();
}

bool UTCServer::start() {
    if (running_) {
        if (logger_) {
            logger_->warn("Server is already running");
        }
        return false;
    }

    if (!config_) {
        UTC_ERROR("UTCServer", "No configuration provided");
        return false;
    }

    if (!open_listeners()) {
        return false;
    }

#ifndef _WIN32
    const bool drop_root = !config_->get_run_as_user().empty() && geteuid() == 0;
#else
    const bool drop_root = false;
#endif
    if (drop_root &&
        !Platform::drop_privileges(config_->get_run_as_user(), config_->get_run_as_group())) {
        UTC_ERROR("UTCServer", "Failed to drop privileges: " + Platform::get_last_error());
        close_listeners();
        return false;
    }

    running_ = true;

    int num_threads = config_->get_worker_threads();
    if (num_threads < 1) {
        num_threads = 1;
    }
    for (int i = 0; i < num_threads; ++i) {
        worker_threads_.emplace_back(&UTCServer::worker_thread_main, this);
    }

    for (int fd : tcp_sockets_) {
        io_threads_.emplace_back(&UTCServer::accept_loop, this, fd);
    }
    for (int fd : udp_sockets_) {
        io_threads_.emplace_back(&UTCServer::udp_loop, this, fd);
    }

    if (logger_) {
        logger_->info("UTC Server started on {}:{} (TCP and UDP, RFC 868)",
                      config_->get_listen_address(), config_->get_listen_port());
    }

    return true;
}

void UTCServer::stop() {
    if (!running_ && tcp_sockets_.empty() && udp_sockets_.empty() && worker_threads_.empty()) {
        return;
    }

    if (logger_ && running_) {
        logger_->info("Stopping UTC Server...");
    }

    running_ = false;
    close_listeners();

    for (auto& thread : io_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    io_threads_.clear();

    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto& connection : connections_) {
            if (connection) {
                connection->close_connection();
            }
        }
        connections_.clear();
    }

    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();

    if (logger_) {
        logger_->info("UTC Server stopped");
    }
}

void UTCServer::accept_loop(int fd) {
    while (running_) {
        if (!wait_readable(fd)) {
            continue;
        }

        sockaddr_storage client_addr{};
        socklen_t client_len = sizeof(client_addr);
        const int client_fd = static_cast<int>(::accept(fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len));
        if (client_fd < 0) {
            if (running_) {
                UTC_ERROR("UTCServer", "Failed to accept connection: " + Platform::get_last_error());
            }
            continue;
        }

        const std::string client_address = format_address(client_addr);
        if (active_connections_ >= config_->get_max_connections()) {
            if (logger_) {
                logger_->warn("Connection limit reached, rejecting {}", client_address);
            }
            Platform::close_socket(client_fd);
            continue;
        }

        auto connection = std::make_unique<UTCConnection>(client_fd, client_address, config_, logger_);
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_.push_back(std::move(connection));
        }

        active_connections_++;
        total_connections_++;
        if (performance_metrics_) {
            performance_metrics_->update_total_connections(total_connections_.load());
            performance_metrics_->update_active_connections(active_connections_.load());
        }
    }
}

void UTCServer::udp_loop(int fd) {
    while (running_) {
        if (!wait_readable(fd)) {
            continue;
        }

        sockaddr_storage client_addr{};
        socklen_t client_len = sizeof(client_addr);
        char discard[64];
        const ssize_t received = ::recvfrom(fd, discard, sizeof(discard), 0,
                                            reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (received < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (running_) {
                UTC_ERROR("UTCServer", "UDP receive failed: " + Platform::get_last_error());
            }
            continue;
        }

        packets_received_++;
        send_udp_reply(fd, reinterpret_cast<sockaddr*>(&client_addr), client_len,
                       format_address(client_addr));
    }
}

void UTCServer::handle_connection(std::unique_ptr<UTCConnection> connection) {
    if (!connection) {
        return;
    }

    auto start_time = std::chrono::steady_clock::now();
    if (performance_metrics_) {
        performance_metrics_->record_request();
    }

    const std::string client_address = connection->get_client_address();
    if (!client_allowed(client_address)) {
        if (logger_) {
            logger_->warn("Denied TCP client {}", client_address);
        }
        connection->close_connection();
        active_connections_--;
        return;
    }

    if (rate_limiter_) {
        const auto limit = rate_limiter_->check_limit(client_address);
        if (!limit.allowed) {
            if (logger_) {
                logger_->warn("Rate limit exceeded for {}", client_address);
            }
            connection->close_connection();
            active_connections_--;
            return;
        }
    }

    UTCPacket packet(get_utc_timestamp());
    if (connection->send_packet(packet)) {
        packets_sent_++;
        if (performance_metrics_) {
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            performance_metrics_->record_response(static_cast<uint64_t>(duration.count()));
        }
    } else if (performance_metrics_) {
        performance_metrics_->record_error();
    }

    connection->close_connection();
    active_connections_--;
    if (performance_metrics_) {
        performance_metrics_->update_active_connections(active_connections_.load());
    }
}

void UTCServer::worker_thread_main() {
    while (true) {
        std::unique_ptr<UTCConnection> connection;
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            if (!connections_.empty()) {
                connection = std::move(connections_.front());
                connections_.erase(connections_.begin());
            }
        }

        if (connection) {
            handle_connection(std::move(connection));
        } else if (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } else {
            break;
        }
    }
}

bool UTCServer::client_allowed(const std::string& client_address) const {
    return !config_ || config_->is_client_permitted(client_address);
}

void UTCServer::send_udp_reply(int fd, const sockaddr* address, socklen_t address_len,
                               const std::string& client_address) {
    if (!client_allowed(client_address)) {
        if (logger_) {
            logger_->warn("Denied UDP client {}", client_address);
        }
        return;
    }

    if (rate_limiter_) {
        const auto limit = rate_limiter_->check_limit(client_address);
        if (!limit.allowed) {
            if (logger_) {
                logger_->warn("Rate limit exceeded for {}", client_address);
            }
            return;
        }
    }

    UTCPacket packet(get_utc_timestamp());
    const std::vector<uint8_t> bytes = packet.to_bytes();
    const ssize_t sent = ::sendto(fd, reinterpret_cast<const char*>(bytes.data()),
                                  static_cast<int>(bytes.size()), 0,
                                  address, address_len);
    if (sent == static_cast<ssize_t>(bytes.size())) {
        packets_sent_++;
    } else if (logger_) {
        logger_->warn("Failed to send UDP time to {}", client_address);
    }
}

bool UTCServer::open_listeners() {
    const std::string& address = config_->get_listen_address();
    const int port = config_->get_listen_port();
    const bool address_is_ipv6 = address.find(':') != std::string::npos && address != "0.0.0.0";
    const bool want_v4 = !address_is_ipv6;
    const bool want_v6 = config_->is_ipv6_enabled() &&
                         (address.empty() || address == "0.0.0.0" || address == "::" || address_is_ipv6);

    auto open_one = [&](int family, int type, bool stream) -> int {
        int fd = Platform::create_socket(family, type, 0);
        if (fd < 0) {
            return -1;
        }
        set_reuseaddr(fd);
        bool bound = false;
        if (family == AF_INET) {
            bound = Platform::bind_socket(fd, address == "::" ? "0.0.0.0" : address, port);
        } else {
            bound = bind_ipv6(fd, address, port);
        }
        if (!bound) {
            Platform::close_socket(fd);
            return -1;
        }
        if (stream && !Platform::listen_socket(fd, config_->get_max_connections())) {
            Platform::close_socket(fd);
            return -1;
        }
        return fd;
    };

    if (want_v4) {
        int tcp = open_one(AF_INET, SOCK_STREAM, true);
        int udp = open_one(AF_INET, SOCK_DGRAM, false);
        if (tcp < 0 || udp < 0) {
            UTC_ERROR("UTCServer", "Failed to bind IPv4 listeners on port " + std::to_string(port) +
                                       ": " + Platform::get_last_error());
            close_listeners();
            if (tcp >= 0) {
                Platform::close_socket(tcp);
            }
            if (udp >= 0) {
                Platform::close_socket(udp);
            }
            return false;
        }
        tcp_sockets_.push_back(tcp);
        udp_sockets_.push_back(udp);
    }

    if (want_v6) {
        int tcp = open_one(AF_INET6, SOCK_STREAM, true);
        int udp = open_one(AF_INET6, SOCK_DGRAM, false);
        if (tcp < 0 || udp < 0) {
            if (logger_) {
                logger_->warn("IPv6 listeners unavailable: {}", Platform::get_last_error());
            }
            if (tcp >= 0) {
                Platform::close_socket(tcp);
            }
            if (udp >= 0) {
                Platform::close_socket(udp);
            }
        } else {
            tcp_sockets_.push_back(tcp);
            udp_sockets_.push_back(udp);
        }
    }

    if (tcp_sockets_.empty() || udp_sockets_.empty()) {
        UTC_ERROR("UTCServer", "No listeners opened");
        close_listeners();
        return false;
    }
    return true;
}

void UTCServer::close_listeners() {
    for (int fd : tcp_sockets_) {
        Platform::close_socket(fd);
    }
    for (int fd : udp_sockets_) {
        Platform::close_socket(fd);
    }
    tcp_sockets_.clear();
    udp_sockets_.clear();
}

uint32_t UTCServer::get_utc_timestamp() {
    return UTCPacket::get_current_utc_timestamp();
}

void UTCServer::update_reference_time() {
    // RFC 868 serves the host clock. Keep the host synced with the OS time
    // service (chrony, systemd-timesyncd, or simple-ntpd).
    if (logger_) {
        logger_->debug("Serving host clock as RFC 868 time");
    }
}

bool UTCServer::reload_config(const std::string& config_file) {
    if (!config_) {
        return false;
    }

    UTCConfig temp_config;
    if (!temp_config.load(config_file)) {
        if (logger_) {
            logger_->error("Failed to load configuration file for reload: {}", config_file);
        }
        return false;
    }

    if (!temp_config.validate()) {
        if (logger_) {
            logger_->error("Configuration validation failed");
            for (const auto& error : temp_config.get_validation_errors()) {
                logger_->error("  - {}", error);
            }
        }
        return false;
    }

    *config_ = temp_config;
    config_->load_from_environment();

    if (rate_limiter_) {
        rate_limiter_->set_enabled(config_->is_rate_limit_enabled());
        if (config_->get_rate_limit_rps() > 0) {
            rate_limiter_->set_rate(static_cast<uint64_t>(config_->get_rate_limit_rps()));
        }
        if (config_->get_rate_limit_burst() > 0) {
            rate_limiter_->set_burst_size(static_cast<uint64_t>(config_->get_rate_limit_burst()));
        }
    }

    if (logger_) {
        logger_->info("Configuration reloaded successfully from: {}", config_file);
    }
    return true;
}

}  // namespace simple_utcd
