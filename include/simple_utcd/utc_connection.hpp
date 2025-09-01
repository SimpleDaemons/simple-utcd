#pragma once

#include <memory>
#include <string>
#include <atomic>
#include "utc_packet.hpp"

namespace simple_utcd {

class UTCConfig;
class Logger;

class UTCConnection {
public:
    UTCConnection(int socket_fd, const std::string& client_address, 
                  UTCConfig* config, Logger* logger);
    ~UTCConnection();
    
    bool is_connected() const { return connected_; }
    const std::string& get_client_address() const { return client_address_; }
    int get_socket_fd() const { return socket_fd_; }
    
    bool send_packet(const UTCPacket& packet);
    bool receive_packet(UTCPacket& packet);
    void close_connection();
    
    // Connection statistics
    int get_packets_sent() const { return packets_sent_; }
    int get_packets_received() const { return packets_received_; }
    int get_bytes_sent() const { return bytes_sent_; }
    int get_bytes_received() const { return bytes_received_; }

private:
    int socket_fd_;
    std::string client_address_;
    UTCConfig* config_;
    Logger* logger_;
    
    std::atomic<bool> connected_;
    std::atomic<int> packets_sent_;
    std::atomic<int> packets_received_;
    std::atomic<int> bytes_sent_;
    std::atomic<int> bytes_received_;
    
    bool send_data(const void* data, size_t size);
    bool receive_data(void* data, size_t size);
    bool is_client_allowed() const;
    bool is_client_denied() const;
};

} // namespace simple_utcd
