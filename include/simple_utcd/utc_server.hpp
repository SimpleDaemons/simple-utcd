#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include "utc_config.hpp"
#include "logger.hpp"

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
    
    // Configuration access
    UTCConfig* get_config() const { return config_; }
    Logger* get_logger() const { return logger_; }

private:
    UTCConfig* config_;
    Logger* logger_;
    
    std::atomic<bool> running_;
    std::vector<std::unique_ptr<UTCConnection>> connections_;
    std::vector<std::thread> worker_threads_;
    
    // Statistics
    std::atomic<int> active_connections_;
    std::atomic<int> total_connections_;
    std::atomic<int> packets_sent_;
    std::atomic<int> packets_received_;
    
    // Server socket
    int server_socket_;
    
    void accept_connections();
    void handle_connection(std::unique_ptr<UTCConnection> connection);
    void worker_thread_main();
    bool create_server_socket();
    void close_server_socket();
    
    // UTC time handling
    uint32_t get_utc_timestamp();
    void update_reference_time();
};

} // namespace simple_utcd
