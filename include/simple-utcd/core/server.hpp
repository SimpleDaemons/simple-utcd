/*
 * includes/simple_utcd/utc_server.hpp
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

#pragma once

#include <cstdint>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif
#include "simple-utcd/config/config.hpp"
#include "simple-utcd/utils/logger.hpp"
#include "simple-utcd/utils/metrics.hpp"
#include "simple-utcd/utils/health_check.hpp"
#include "simple-utcd/network/async_io.hpp"
#include "simple-utcd/security/rate_limiter.hpp"

namespace simple_utcd {

class UTCConnection;
class UTCPacket;

class UTCServer {
public:
    UTCServer(UTCConfig* config, Logger* logger);
    ~UTCServer();

    bool start();
    void stop();
    bool is_running() const { return running_; }

    // Server statistics
    int get_active_connections() const { return active_connections_; }
    int get_total_connections() const { return total_connections_; }
    int get_packets_sent() const { return packets_sent_; }
    int get_packets_received() const { return packets_received_; }
    
    // Metrics and health
    class PerformanceMetrics* get_performance_metrics() const { return performance_metrics_.get(); }
    class HealthChecker* get_health_checker() const { return health_checker_.get(); }

    // Configuration access
    UTCConfig* get_config() const { return config_; }
    Logger* get_logger() const { return logger_; }
    
    // Dynamic configuration reloading
    bool reload_config(const std::string& config_file);

private:
    UTCConfig* config_;
    Logger* logger_;

    std::atomic<bool> running_;
    std::vector<std::unique_ptr<UTCConnection>> connections_;
    std::vector<std::thread> worker_threads_;
    std::vector<std::thread> io_threads_;
    std::mutex connections_mutex_;

    // Statistics
    std::atomic<int> active_connections_;
    std::atomic<int> total_connections_;
    std::atomic<int> packets_sent_;
    std::atomic<int> packets_received_;

    // Listening sockets. TCP uses accept; UDP answers each datagram in place.
    std::vector<int> tcp_sockets_;
    std::vector<int> udp_sockets_;
    std::unique_ptr<RateLimiter> rate_limiter_;
    
    // Metrics and health checking
    std::unique_ptr<PerformanceMetrics> performance_metrics_;
    std::unique_ptr<HealthChecker> health_checker_;
    
    // Async I/O support
    std::unique_ptr<AsyncIOManager> async_io_manager_;

    void accept_loop(int fd);
    void udp_loop(int fd);
    void handle_connection(std::unique_ptr<UTCConnection> connection);
    void worker_thread_main();
    bool open_listeners();
    void close_listeners();
    bool client_allowed(const std::string& client_address) const;
    void send_udp_reply(int fd, const struct sockaddr* address, socklen_t address_len,
                        const std::string& client_address);

    // UTC time handling
    uint32_t get_utc_timestamp();
    void update_reference_time();
};

} // namespace simple_utcd
